/*
* Copyright (C) 2022 Gemini
* ===============================================================
* AIX reader module
* ---------------------------------------------------------------
* Code to open and parse AIX files.
* ===============================================================
*/
#include "criware.h"

int OpenAIX(const char* filename, AIX_Demuxer**obj)
{
	*obj = nullptr;

	HANDLE fp = ADXF_OpenFile(filename);
	if (fp == INVALID_HANDLE_VALUE)
		return 0;

	AIX_Demuxer* aix = new AIX_Demuxer;
	aix->fp = fp;

	// parse header and data
	AIX_HEADER head;
	ADXF_ReadFile(fp, &head, sizeof(head));

	aix->Open(fp, head.stream_count, head.data_size.dw());

	for (DWORD i = 0; i < head.stream_count; i++)
	{
		ADX_header_AIX adx_head;

		auto s = &aix->stream[i];
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

		ADXDEC_SetCoeff(s);
	}

	*obj = aix;
	return 1;
}
