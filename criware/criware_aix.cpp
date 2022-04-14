/*
* Copyright (C) 2022 Gemini
* ===============================================================
* AIX reader module
* ---------------------------------------------------------------
* Code to open and parse AIX files.
* ===============================================================
*/
#include "criware.h"

int OpenAIX(const char* filename, AIX_Handle**obj)
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

	parent->Open(fp, head.stream_count, head.data_size.dw());

	for (DWORD i = 0; i < head.stream_count; i++)
	{
		ADX_header_AIX adx_head;

		auto s = &parent->stream[i];
		s->Read(&adx_head, sizeof(adx_head));

		s->block_size = adx_head.block_size;
		s->channel_count = adx_head.channel_count;
		s->highpass_frequency = adx_head.highpass_frequency.w();
		s->sample_bitdepth = adx_head.sample_bitdepth;
		s->sample_rate = adx_head.sample_rate.dw();
		s->total_samples = adx_head.total_samples.dw();
		s->copyright_offset = adx_head.copyright_offset.w();

		s->loop_enabled = 1;
		s->loop_start_index = 0;
		s->loop_end_index = s->total_samples;

		adx_set_coeff(s);
	}

	*obj = aix;
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