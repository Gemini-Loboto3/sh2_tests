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
	chunk_pos = new u_long[stream_count];

	stream = new AIXStream[stream_count];
	for (u_long i = 0; i < stream_count; i++)
	{
		stream[i].parent = this;
		stream[i].stream_id = i;
	}

#if INTERLEAVE_FILE
	// generate a list of where chunks are stored for each channel 0
	AIX_CHUNK chunk;
	AIXP_HEADER aixp;
	for (int i = 0, loop = 1; loop;)
	{
		DWORD read;
		ReadFile(fp, &chunk, sizeof(chunk), &read, nullptr);

		switch (chunk.type)
		{
		case 'P':
			ReadFile(fp, &aixp, sizeof(aixp), &read, nullptr);
			if (aixp.stream_id == 0)
				chunk_pos[i++] = SetFilePointer(fp, 0, nullptr, FILE_CURRENT) - sizeof(chunk) - sizeof(aixp);
			SetFilePointer(fp, chunk.next.dw() - sizeof(aixp), nullptr, FILE_CURRENT);
			break;
		case 'E':
			loop = 0;
		}
	}
#else
	AIX_CHUNK chunk;
	AIXP_HEADER aixp;

	data = new BYTE*[stream_count];
	pos = new BYTE*[stream_count];
	for (u_long i = 0; i < stream_count; i++)
	{
		data[i] = new BYTE[total_size / stream_count];	// allocate a buffer large enough to store deinterleaved data
		pos[i] = data[i];
	}

	for (int i = 0, loop = 1; loop;)
	{
		DWORD read;
		ReadFile(fp, &chunk, sizeof(chunk), &read, nullptr);

		switch (chunk.type)
		{
		case 'P':
			ReadFile(fp, &aixp, sizeof(aixp), &read, nullptr);
			ReadFile(fp, pos[aixp.stream_id], chunk.next.dw() - sizeof(aixp), &read, nullptr);
			pos[aixp.stream_id] += chunk.next.dw() - sizeof(aixp);
			break;
		case 'E':
			loop = 0;
		}
	}

	for (u_long i = 0; i < stream_count; i++)
		pos[i] = data[i];
#endif
}

void AIXParent::Close()
{
	if (stream_count)
		delete[] stream;

#if !INTERLEAVE_FILE
	delete[] pos;
	for (u_long i = 0; i < stream_count; i++)
		delete[] data[i];
	delete[] data;
#endif

	CloseHandle(fp);
}

void AIXParent::RequestData(u_long stream_id, void* data, u_long size)
{
#if !INTERLEAVE_FILE
	memcpy(data, pos[stream_id], size);
	pos[stream_id] += size;
#endif
}

void AIXParent::RequestSeek(u_long stream_id, u_long pos)
{
#if !INTERLEAVE_FILE
	this->pos[stream_id] = &data[stream_id][pos];
#endif
}

void AIXStream::Read(void* buffer, size_t size)
{
#if !INTERLEAVE_FILE
	parent->RequestData(stream_id, buffer, size);
#endif
}

void AIXStream::Seek(u_long pos, u_long mode)
{
#if !INTERLEAVE_FILE
	parent->RequestSeek(stream_id, pos);
#endif
}
