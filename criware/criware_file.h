#pragma once

#define STR_ADX_CACHING		0		// caching switch for ADX
#define STR_AIX_CACHING		0		// caching switch for AIX
#define STREAM_CACHE_SIZE	2048

#define AIX_SEGMENTED		1		// set to 1 for caching on demand

// file helpers
HANDLE ADXF_OpenFile(const char* filename);
void ADXF_CloseFile(HANDLE fp);
void ADXF_ReadFile(HANDLE fp, void* buffer, size_t size);

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
	ADXStream() : fp(nullptr),
		start(0)
#if STR_ADX_CACHING
		, cache{0},
		pos_cache(0),
		last_pos(-1)
#endif
	{}

	virtual ~ADXStream()
	{
		Close();
	}

	int Open(const char* filename);
	int Open(HANDLE fp, u_long pos);
	void Close();

	virtual void Read(void* buffer, size_t size);
	virtual void Seek(u_long pos, u_long mode);

	HANDLE fp;
	u_long start;
#if STR_ADX_CACHING
	BYTE cache[STREAM_CACHE_SIZE];
	u_long pos_cache, last_pos;
#endif
};

// AIX streaming interface, a hell of caching and deinterleaving
class AIXStream;

class AIX_Demuxer
{
public:
	AIX_Demuxer() : fp(INVALID_HANDLE_VALUE),
		stream(nullptr),
		stream_count(0)
#if STR_AIX_CACHING
		, cache{0},
		pos_cache(0)
#endif
	{

	}

	virtual ~AIX_Demuxer()
	{
		Close();
	}

	void Open(HANDLE fp, u_long stream_count, u_long total_size);
	void Close();
	void Read(void* buffer, size_t size);

#if STR_AIX_CACHING
	void InitCache();
#endif
	void RequestData(u_long count);

	HANDLE fp;
	AIXStream* stream;
	u_long stream_count;
#if STR_AIX_CACHING
	BYTE cache[STREAM_CACHE_SIZE];
	u_long pos_cache;
#endif
};

class AIXStream : public CriFileStream
{
public:
	AIXStream() : parent(nullptr),
		stream_id(0),
		pos(0),
		cached(0),
		data(nullptr)
	{
	}
	virtual ~AIXStream()
	{
		pos = 0;
		if (data)
		{
			delete[] data;
			data = nullptr;
		}
	}

	void MakeBuffer(u_long size)
	{
		data = new BYTE[size];
	}

	virtual void Read(void* buffer, size_t size);
	virtual void Seek(u_long pos, u_long mode);

	AIX_Demuxer* parent;
	u_long stream_id,
		pos,
		cached;
	BYTE* data;
};
