/*
* Copyright (C) 2022 Gemini
* ===============================================================
* AIX reader module
* ---------------------------------------------------------------
* Code to open and parse AIX files.
* ===============================================================
*/
#include "criware.h"
#include <chrono>

#define MEASURE_ACCESS		1

#if MEASURE_ACCESS
DWORD TimeGetTime()
{
	return (DWORD)(std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 1000.);
}
#endif

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

typedef struct AIX_THREAD_CTX
{
	AIXP_Object* obj;
	const char* fname;
} AIX_THREAD_CTX;

static DWORD WINAPI aix_load_thread(LPVOID param)
{
#if MEASURE_ACCESS
	DWORD start = TimeGetTime();
	OutputDebugStringA(__FUNCTION__ ": preparing AIX...\n");
#endif

	auto obj = (AIXP_Object*)param;

	AIX_Demuxer* aix;
	if (OpenAIX(obj->fname, &aix) == 0)
	{
		obj->state = AIXP_STAT_ERROR;
		return 0;
	}
	obj->aix = aix;
	obj->stream_no = aix->stream_count;

	// create the necessary buffers
	for (int i = 0; i < obj->stream_no; i++)
	{
		obj->adxt[i].state = ADXT_STAT_PREP;
		obj->adxt[i].stream = &aix->stream[i];
		obj->adxt[i].obj = ds_FindObj();
		obj->adxt[i].obj->loops = true;
		obj->adxt[i].obj->adx = &obj->adxt[i];
		obj->adxt[i].obj->CreateBuffer(obj->adxt[i].stream);
	}
	// flag any unused adxt as stopped
	for (int i = obj->stream_no; i < _countof(obj->adxt); i++)
		obj->adxt[i].state = ADXT_STAT_STOP;

	obj->state = AIXP_STAT_PLAYING;
	// kick all playback for all streams at once
	for (int i = 0; i < obj->stream_no; i++)
	{
		obj->adxt[i].state = ADXT_STAT_PLAYING;
		obj->adxt[i].obj->Play();
	}

#if MEASURE_ACCESS
	char str[64];
	sprintf_s(str, sizeof(str), __FUNCTION__ ": AIX done parsing in %dms, play!\n", TimeGetTime() - start);
	OutputDebugStringA(str);
#endif

	obj->th = 0;

	return 0;
}

void aix_start(AIXP_Object* obj, const char* fname)
{
	obj->state = AIXP_STAT_PREP;
	obj->fname = fname;
	for (int i = 0; i < _countof(obj->adxt); i++)
	{
		obj->adxt[i].state = ADXT_STAT_DECINFO;
		obj->adxt[i].is_aix = 1;
	}

	// just in case two AIX try to boot on the same object
	if (obj->th)
		WaitForSingleObject(obj->th, INFINITE);

	obj->th = CreateThread(nullptr, 0, aix_load_thread, obj, 0, nullptr);
}

void AIXP_Object::Release()
{
	if (state != AIXP_STAT_STOP)
	{
		for (int i = 0; i < stream_no; i++)
		{
			adxt[i].obj->Stop();
			adxt[i].obj->Release();
			adxt[i].obj = nullptr;
		}
		stream_no = 0;
		memset(adxt, 0, sizeof(adxt));

		if (aix)
		{
			delete aix;
			aix = nullptr;
		}

		state = AIXP_STAT_STOP;
	}
}
