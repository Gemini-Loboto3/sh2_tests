#pragma once

#define INTERLEAVE_FILE		0

// generic streaming interface
class CriFileStream
{
public:
	CriFileStream() :
		copyright_offset(0),
		block_size(0),
		sample_bitdepth(0),
		channel_count(0),
		sample_rate(0),
		total_samples(0),
		highpass_frequency(0),
		loop_enabled(0),
		loop_start_index(0),
		loop_end_index(0),
		past_samples{ 0 },
		sample_index(0),
		coefficient{ 0, 0 }
	{}
	virtual ~CriFileStream() {}

	virtual void Read(void* buffer, size_t size) {}
	virtual void Seek(u_long pos, u_long mode) {}

	u_long copyright_offset,
		block_size,
		sample_bitdepth,
		channel_count,
		sample_rate,
		total_samples,
		highpass_frequency,
		loop_enabled,
		loop_start_index,
		loop_end_index;
	short past_samples[16];
	u_long sample_index;
	short coefficient[2];
};

// ADX streaming interface, very simple
class ADXStream : public CriFileStream
{
public:
	virtual ~ADXStream()
	{
		Close();
	}

	int Open(const char* filename);
	int Open(HANDLE fp);
	void Close();

	virtual void Read(void* buffer, size_t size);
	virtual void Seek(u_long pos, u_long mode);

	HANDLE fp;
	u_long start;
};

// AIX streaming interface, a hell of caching and deinterleaving
class AIXStream;

class AIXParent
{
public:
	virtual ~AIXParent()
	{
		Close();
	}

	void Open(HANDLE fp, u_long stream_count, u_long total_size);
	void Close();

	void RequestData(u_long stream_id, void* data, u_long size);
	void RequestSeek(u_long stream_id, u_long pos);

	HANDLE fp;
	AIXStream* stream;
	u_long stream_count;
	u_long *chunk_pos;
#if !INTERLEAVE_FILE
	BYTE** data;
	BYTE** pos;
#endif
};

#define AIX_BUFFER_SIZE	2048

class AIXStream : public CriFileStream
{
public:
	AIXStream() : parent(nullptr),
		consumed(0),
		available(0),
		//buffer{0},
		stream_id(0)
	{
	}
	virtual ~AIXStream()
	{}

	virtual void Read(void* buffer, size_t size);
	virtual void Seek(u_long pos, u_long mode);

	AIXParent* parent;
	u_long consumed,
		available,
		stream_id;
	//BYTE buffer[AIX_BUFFER_SIZE];
};
