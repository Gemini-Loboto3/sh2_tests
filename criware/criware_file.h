#pragma once

class CriFileStream
{
public:
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
	int* past_samples;			// Previously decoded samples from each channel, zeroed at start (size = 2*channel_count)
	u_long sample_index;		// sample_index is the index of sample set that needs to be decoded next
	double coefficient[2];
};

class ADXStream : public CriFileStream
{
public:
	int Open(const char* filename);
	void Close();

	virtual void Read(void* buffer, size_t size);
	virtual void Seek(u_long pos, u_long mode);

	HANDLE fp;
};

class AIXStream;

class AIXParent
{
public:
	void Open(HANDLE fp, u_long stream_no);
	void Close();

	HANDLE fp;
	AIXStream* streams;
	u_long stream_no;
};

class AIXStream : public CriFileStream
{
public:
	virtual void Read(void* buffer, size_t size);
	virtual void Seek(u_long pos, u_long mode);

	AIXParent* parent;
	BYTE buffer[2048];
};
