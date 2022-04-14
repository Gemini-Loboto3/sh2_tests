#include "criware.h"
#include "criware_file.h"

// ------------------------------------------------
// ADX stream code, normal file wrapping
int ADXStream::Open(const char* filename)
{
	fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fp == INVALID_HANDLE_VALUE)
		return S_FALSE;

	start = 0;
	return S_OK;
}

int ADXStream::Open(HANDLE _fp)
{
	fp = _fp;
	start = SetFilePointer(fp, 0, nullptr, FILE_CURRENT);

	return S_OK;
}

void ADXStream::Close()
{
	CloseHandle(fp);
}

void ADXStream::Read(void* buffer, size_t size)
{
	DWORD read;
	ReadFile(fp, buffer, size, &read, nullptr);
}

void ADXStream::Seek(u_long pos, u_long mode)
{
	SetFilePointer(fp, pos + start, nullptr, mode);
}

// ------------------------------------------------
// AIX stream code, a lot of shit going on here
void AIXParent::Open(HANDLE fp, u_long stream_count, u_long total_size)
{
	this->fp = fp;
	this->stream_count = stream_count;
	chunk_pos = new u_long[stream_count];

	streams = new AIXStream[stream_count];
	for (u_long i = 0; i < stream_count; i++)
	{
		streams[i].parent = this;
		streams[i].stream_id = i;
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
		delete[] streams;

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
