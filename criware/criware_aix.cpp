#include "criware.h"
#include <vector>

typedef struct AIX_Handle
{
	AIXParent *parent;
} AIX_HANDLE;

int OpenAIX(const char* filename, AIX**obj)
{
	*obj = nullptr;

	HANDLE fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fp == INVALID_HANDLE_VALUE)
		return 0;

	AIXParent* parent = new AIXParent;
	parent->fp = fp;

	AIX_Handle* aix = new AIX_Handle;
	aix->parent = parent;

	// parse header and data
	AIX_HEADER head;
	DWORD read;
	ReadFile(fp, &head, sizeof(head), &read, nullptr);

	for (DWORD i = 0; i < head.stream_no.dw(); i++)
	{
		ADX_header_AIX adx_head;

		auto s = &parent->streams[i];
		s->Read(&adx_head, sizeof(adx_head));

		s->block_size = adx_head.block_size;
		s->channel_count = adx_head.channel_count;
		s->highpass_frequency = adx_head.highpass_frequency.w();
		s->sample_bitdepth = adx_head.sample_bitdepth;
		s->sample_rate = adx_head.sample_rate.dw();
		s->total_samples = adx_head.total_samples.dw();
		s->copyright_offset = adx_head.copyright_offset.w();
	}

	*obj = (AIX*)aix;
	return 1;
}

void CloseAIX(AIX* obj)
{
	AIX_Handle* aix = (AIX_Handle*)obj;

	if (aix->parent)
	{
		aix->parent->Close();
		delete aix->parent;
		aix = nullptr;
	}

	delete obj;
}