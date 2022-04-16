/*
* Copyright (C) 2022 Gemini
* ===============================================================
* File streaming interfaces
* ---------------------------------------------------------------
* Classes to manage ADX and AIX streaming to a sound object, also
* common Windows file handle operations.
* ===============================================================
*/
#include "criware.h"

// ------------------------------------------------
// Helpers for debloating file code
// ------------------------------------------------
HANDLE ADXF_OpenFile(const char* filename)
{
	return CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
}

void ADXF_CloseFile(HANDLE fp)
{
	CloseHandle(fp);
}

void ADXF_ReadFile(HANDLE fp, void* buffer, size_t size)
{
	DWORD read;
	ReadFile(fp, buffer, size, &read, nullptr);
#if _DEBUG
	if (read != size)
		OutputDebugStringA("Warning: read data is not the same as requested.\n");
#endif
}

// ------------------------------------------------
// ADX stream code, normal file wrapping
// ------------------------------------------------
int ADXStream::Open(const char* filename)
{
	fp = ADXF_OpenFile(filename);
	if (fp == INVALID_HANDLE_VALUE)
		return S_FALSE;

	start = 0;
#if STR_ADX_CACHING
	Seek(0, SEEK_SET);
#endif

	return S_OK;
}

int ADXStream::Open(HANDLE _fp, u_long pos)
{
	fp = _fp;
	start = pos;
#if STR_ADX_CACHING
	pos_cache = 0;
	last_pos = -1;
#endif
	Seek(0, SEEK_SET);

	return S_OK;
}

void ADXStream::Close()
{
	if(start == 0)
		CloseHandle(fp);
}

void ADXStream::Read(void* buffer, size_t size)
{
#if STR_ADX_CACHING
	BYTE* dst = (BYTE*)buffer;
	if (size + pos_cache > STREAM_CACHE_SIZE)
	{
		size_t remainder = STREAM_CACHE_SIZE - pos_cache;
		memcpy(dst, &cache[pos_cache], remainder);
		dst += remainder;
		size -= remainder;
		ADXF_ReadFile(fp, cache, STREAM_CACHE_SIZE);
		pos_cache = 0;
		last_pos++;
	}

	memcpy(dst, &cache[pos_cache], size);
	pos_cache += size;
#else
	ADXF_ReadFile(fp, buffer, size);
#endif
}

void ADXStream::Seek(u_long pos, u_long mode)
{
#if STR_ADX_CACHING
	u_long npos = (pos + start) / STREAM_CACHE_SIZE;
	if (npos != last_pos)
	{
		DWORD read;
		SetFilePointer(fp, npos * STREAM_CACHE_SIZE, nullptr, mode);
		ADXF_ReadFile(fp, cache, STREAM_CACHE_SIZE, &read, nullptr);
		last_pos = npos;
	}
	pos_cache = (pos + start) % STREAM_CACHE_SIZE;
#else
	SetFilePointer(fp, pos + start, nullptr, mode);
#endif
}

// ------------------------------------------------
// AIX demux & stream code
// ------------------------------------------------
void AIX_Demuxer::Open(HANDLE _fp, u_long _stream_count, u_long total_size)
{
	fp = _fp;
	stream_count = _stream_count;

	stream = new AIXStream[stream_count];
	for (u_long i = 0; i < stream_count; i++)
	{
		stream[i].parent = this;
		stream[i].stream_id = i;
		stream[i].MakeBuffer(total_size / stream_count);
	}

#if STR_AIX_CACHING
	InitCache();
#endif

#if AIX_SEGMENTED
	// request the necessary amount of data for header and a chunk of ADX
	RequestData(2);
#else
	AIX_CHUNK chunk;
	AIXP_HEADER aixp;

	AIXStream* s;

	for (int i = 0, loop = 1; loop;)
	{
		Read(&chunk, sizeof(chunk));

		switch (chunk.type)
		{
		case 'P':
			Read(&aixp, sizeof(aixp));
			s = &stream[aixp.stream_id];
			Read(&s->data[s->cached], chunk.next.dw() - sizeof(aixp));
			s->cached += chunk.next.dw() - sizeof(aixp);
			break;
		case 'E':
			loop = 0;
		}
	}
#endif
}

void AIX_Demuxer::Close()
{
	if (stream_count)
		delete[] stream;
	stream_count = 0;
	stream = nullptr;

	CloseHandle(fp);
	fp = INVALID_HANDLE_VALUE;
}

#if STR_AIX_CACHING
void AIX_Demuxer::InitCache()
{
	pos_cache = 0;
	ADXF_ReadFile(fp, cache, STREAM_CACHE_SIZE);
}
#endif

void AIX_Demuxer::Read(void* buffer, size_t size)
{
	BYTE* dst = (BYTE*)buffer;
#if STR_AIX_CACHING
	// cache overflow
	if (size + pos_cache > STREAM_CACHE_SIZE)
	{
		size_t remainder = STREAM_CACHE_SIZE - pos_cache;	// how much we can still read
		memcpy(dst, &cache[pos_cache], remainder);			// send whatever is left in the buffer
		dst += remainder;									// skip what is stored
		size -= remainder;
		// check if next read fits in the cache
		if (size > STREAM_CACHE_SIZE)
		{
			size_t blocks = size / STREAM_CACHE_SIZE;
			for (u_long i = 0; i < blocks; i++, dst += STREAM_CACHE_SIZE, size -= STREAM_CACHE_SIZE)
				ADXF_ReadFile(fp, dst, STREAM_CACHE_SIZE);
		}
		// refill the cache
		pos_cache = 0;
		ADXF_ReadFile(fp, cache, STREAM_CACHE_SIZE);		// cache more
	}

	if (size)
	{
		memcpy(dst, &cache[pos_cache], size);
		pos_cache += size;
	}
#else
	ADXF_ReadFile(fp, buffer, size);
#endif
}

void AIX_Demuxer::RequestData(u_long count)
{
#if AIX_SEGMENTED
	AIX_CHUNK chunk;
	AIXP_HEADER aixp;
	AIXStream* s;

	for (u_long i = 0; i < stream_count * count; i++)
	{
		Read(&chunk, sizeof(chunk));

		switch (chunk.type)
		{
		case 'P':
			Read(&aixp, sizeof(aixp));
			s = &stream[aixp.stream_id];
			Read(&s->data[s->cached], chunk.next.dw() - sizeof(aixp));
			s->cached += chunk.next.dw() - sizeof(aixp);
			break;
		case 'E':	// end parsing
			return;
		}
	}
#endif
}

// ------------------------------------------------
// AIX stream, behaves the same as ADX streams but
// from memory
// ------------------------------------------------
void AIXStream::Read(void* buffer, size_t size)
{
#if !AIX_SEGMENTED
	memcpy(buffer, &data[pos], size);
	pos += size;
#else
	// we're reading ahead of what's cached from AIX
	if (pos + size > cached)
		parent->RequestData(1);	// 1 whole read should be safe
	// still inside cache, just copy over
	memcpy(buffer, &data[pos], size);
	pos += size;
#endif
}

void AIXStream::Seek(u_long _pos, u_long mode)
{
#if !AIX_SEGMENTED
	pos = _pos;
#else
	while(_pos > cached)
		parent->RequestData(1);	// request until we're good
	pos = _pos;
#endif
}
