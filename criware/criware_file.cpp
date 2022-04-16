/*
* Copyright (C) 2022 Gemini
* ===============================================================
* File streaming interfaces
* ---------------------------------------------------------------
* Classes to manage ADX and AIX streaming to a sound object.
* 
* TODO: The AIX module could need some rewriting, as the code
* isn't exactly the best and most efficient.
* ===============================================================
*/
#include "criware.h"

// ------------------------------------------------
// ADX stream code, normal file wrapping
int ADXStream::Open(const char* filename)
{
	fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fp == INVALID_HANDLE_VALUE)
		return S_FALSE;

	start = 0;
	Seek(0, SEEK_SET);

	return S_OK;
}

int ADXStream::Open(HANDLE _fp, u_long pos)
{
	fp = _fp;
	start = pos;
#if STREAM_CACHING
	Seek(0, SEEK_SET);
#endif

	return S_OK;
}

void ADXStream::Close()
{
	if(start == 0)
		CloseHandle(fp);
}

void ADXStream::Read(void* buffer, size_t size)
{
	DWORD read;
#if STREAM_CACHING
	if (size + pos_cache <= STREAM_CACHE_SIZE)
	{
		memcpy(buffer, &cache[pos_cache], size);
		pos_cache += size;
	}
	else
	{
		BYTE* b = (BYTE*)buffer;
		size_t nsize = (size + pos_cache) % STREAM_CACHE_SIZE;
		memcpy(b, cache, nsize);
		ReadFile(fp, cache, STREAM_CACHE_SIZE, &read, nullptr);
		pos_cache = size - nsize;
		memcpy(&b[nsize], cache, pos_cache);
	}
#else
	ReadFile(fp, buffer, size, &read, nullptr);
#endif
}

void ADXStream::Seek(u_long pos, u_long mode)
{
#if STREAM_CACHING
	u_long npos = (pos + start) / STREAM_CACHE_SIZE;
	if (npos != old_pos)
	{
		DWORD read;
		SetFilePointer(fp, npos * STREAM_CACHE_SIZE, nullptr, mode);
		ReadFile(fp, cache, STREAM_CACHE_SIZE, &read, nullptr);
		old_pos = npos;
	}
	pos_cache = (pos + start) % STREAM_CACHE_SIZE;
#else
	SetFilePointer(fp, pos + start, nullptr, mode);
#endif
}

// ------------------------------------------------
// AIX stream code, a lot of shit going on here
void AIXParent::Open(HANDLE fp, u_long stream_count, u_long total_size)
{
	this->fp = fp;
	this->stream_count = stream_count;

	stream = new AIXStream[stream_count];
	for (u_long i = 0; i < stream_count; i++)
	{
		stream[i].parent = this;
		stream[i].stream_id = i;
		stream[i].MakeBuffer(total_size / stream_count);
	}

#if AIX_DEINTERLEAVE
	RequestData(2);		// request the necessary amount of data for header and a chunk of ADX
#else
	AIX_CHUNK chunk;
	AIXP_HEADER aixp;

	AIXStream* s;

	for (int i = 0, loop = 1; loop;)
	{
		DWORD read;
		ReadFile(fp, &chunk, sizeof(chunk), &read, nullptr);

		switch (chunk.type)
		{
		case 'P':
			ReadFile(fp, &aixp, sizeof(aixp), &read, nullptr);
			s = &stream[aixp.stream_id];
			ReadFile(fp, &s->data[s->cached], chunk.next.dw() - sizeof(aixp), &read, nullptr);
			s->cached += chunk.next.dw() - sizeof(aixp);
			break;
		case 'E':
			loop = 0;
		}
	}
#endif
}

void AIXParent::Close()
{
	if (stream_count)
		delete[] stream;

	CloseHandle(fp);
}

void AIXParent::RequestData(u_long count)
{
#if AIX_DEINTERLEAVE
	AIX_CHUNK chunk;
	AIXP_HEADER aixp;
	AIXStream* s;

	for (int i = 0; i < stream_count * count; i++)
	{
		DWORD read;
		ReadFile(fp, &chunk, sizeof(chunk), &read, nullptr);

		switch (chunk.type)
		{
		case 'P':
			ReadFile(fp, &aixp, sizeof(aixp), &read, nullptr);
			s = &stream[aixp.stream_id];
			ReadFile(fp, &s->data[s->cached], chunk.next.dw() - sizeof(aixp), &read, nullptr);
			s->cached += chunk.next.dw() - sizeof(aixp);
			break;
		case 'E':
			return;
		}
	}
#endif
}

void AIXStream::Read(void* buffer, size_t size)
{
#if !AIX_DEINTERLEAVE
	memcpy(buffer, &data[pos], size);
	pos += size;
#else
	// we're reading ahead of what's cached from AIX
	if (pos + size > cached)
		parent->RequestData(2);	// 2 whole reads should be safe
	// still inside cache, just copy over
	memcpy(buffer, &data[pos], size);
	pos += size;
#endif
}

void AIXStream::Seek(u_long _pos, u_long mode)
{
#if !AIX_DEINTERLEAVE
	pos = _pos;
#else
	if(_pos > cached)
		parent->RequestData(2);	// 2 whole reads should be safe
	pos = _pos;
#endif
}
