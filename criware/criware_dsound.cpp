/*
* Copyright (C) 2022 Gemini
* ===============================================================
* DirectSound interface for buffers
* ---------------------------------------------------------------
* Just an interface with DirectSound8 to generate buffers and
* some helpers.
* ===============================================================
*/
#include "criware.h"

LPDIRECTSOUND8 pDS8;

#define MAX_OBJ			32
#define BUFFER_SIZE		32768
#define BUFFER_HALF		(BUFFER_SIZE / 2)
#define BUFFER_QUART	(BUFFER_HALF / 2)

static SndObj obj_tbl[MAX_OBJ];

void ds_SetupSound(LPDIRECTSOUND8 pDS)
{
	pDS8 = pDS;

	for (int i = 0; i < MAX_OBJ; i++)
		obj_tbl[i].used = 0;
}

SndObj* ds_FindObj()
{
	for (int i = 0; MAX_OBJ; i++)
	{
		if (obj_tbl[i].used == 0)
			return &obj_tbl[i];
	}

	return nullptr;
}

void ds_CreateBuffer(SndObj *obj, CriFileStream* stream)
{
	obj->str = stream;

	obj->fmt.cbSize = sizeof(WAVEFORMATEX);
	obj->fmt.nSamplesPerSec = stream->sample_rate;
	obj->fmt.nBlockAlign = 4;
	obj->fmt.nChannels = (WORD)stream->channel_count;
	obj->fmt.wBitsPerSample = 16;
	obj->fmt.wFormatTag = WAVE_FORMAT_PCM;
	obj->fmt.nAvgBytesPerSec = stream->sample_rate * 2 * stream->channel_count;

	DSBUFFERDESC desc = { 0 };
	desc.dwSize = sizeof(desc);
	desc.lpwfxFormat = &obj->fmt;
	desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | /*DSBCAPS_CTRL3D |*/ DSBCAPS_CTRLFREQUENCY | DSBCAPS_GETCURRENTPOSITION2 /*| DSBCAPS_LOCSOFTWARE*/;
	desc.dwBufferBytes = BUFFER_SIZE;
	if (FAILED(pDS8->CreateSoundBuffer(&desc, &obj->pBuf, nullptr)))
		MessageBoxA(nullptr, "error", __FUNCTION__, MB_OK);

	DWORD bytes1, bytes2;
	short* ptr1, * ptr2;
	obj->pBuf->Lock(0, BUFFER_SIZE, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
	auto needed = decode_adx_standard(stream, ptr1, bytes1 / obj->fmt.nBlockAlign, obj->loops);
	if (needed && obj->loops == 0)
	{
		needed *= obj->fmt.nBlockAlign;
		memset(&ptr1[(bytes1 - needed) / 2], 0, needed);
	}
	obj->pBuf->Unlock(ptr1, bytes1, ptr2, bytes2);

	obj->used = 1;
	obj->offset = 0;
}

u_long ds_GetPosition(SndObj* obj)
{
	DWORD pos;
	obj->pBuf->GetCurrentPosition(&pos, nullptr);

	return pos;
}

void adxds_SendData(SndObj *obj)
{
	obj->trans_lock = 1;

	u_long add = 0, pos;
	const DWORD snd_dwBytes = BUFFER_QUART;

	ds_GetPosition(obj);
	obj->pBuf->GetCurrentPosition(&pos, nullptr);

	if (pos - obj->offset < 0)
		add = BUFFER_SIZE;
	if (pos + add - obj->offset > 2 * snd_dwBytes + 16)
	{
		DWORD bytes1, bytes2;
		short* ptr1, * ptr2;

		ADX_lock();
		obj->pBuf->Lock(obj->offset, snd_dwBytes, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
		auto needed = decode_adx_standard(obj->str, ptr1, bytes1 / obj->fmt.nBlockAlign, obj->loops);
		if (needed && obj->loops == 0)
		{
			needed *= obj->fmt.nBlockAlign;
			memset(&ptr1[(bytes1 - needed) / 2], 0, needed);
		}
		obj->pBuf->Unlock(ptr1, bytes1, ptr2, bytes2);
		ADX_unlock();

		auto total = snd_dwBytes + obj->offset;
		obj->offset = total;
		if (BUFFER_SIZE <= total)
			obj->offset = total - BUFFER_SIZE;
	}

	obj->trans_lock = 0;
}

void ds_SetVolume(SndObj* obj, int vol)
{
	if (vol < -1000)
		vol = -1000;

	obj->volume = vol;
	if (obj->used && obj->pBuf)
		obj->pBuf->SetVolume(vol * 10);
	else obj->set_volume = 1;
}

void ds_Play(SndObj* obj)
{
	if (obj->used && obj->pBuf)
		obj->pBuf->Play(0, 0, DSBPLAY_LOOPING);
}

void ds_Stop(SndObj* obj)
{
	if (obj->used && obj->pBuf)
	{
		obj->pBuf->Stop();

		int s;
		do { s = ds_GetStatus(obj); } while (s == DSOS_LOOPING || s == DSOS_PLAYING);
		obj->stopped = 1;
	}
}

int ds_GetStatus(SndObj* obj)
{
	if (obj->used == 0 || obj->pBuf == nullptr)
		return DSOS_UNUSED;

	DWORD status;
	obj->pBuf->GetStatus(&status);

	if (status & DSBSTATUS_LOOPING)
		return DSOS_LOOPING;
	if (status & DSBSTATUS_PLAYING)
		return DSOS_PLAYING;

	return DSOS_ENDED;
}

void ds_Release(SndObj* obj)
{
	if (obj->used && obj->pBuf)
	{
		while (obj->trans_lock);	// wait if it's transferring data in the thread
		obj->pBuf->Release();
		memset(obj, 0, sizeof(*obj));
	}
}

void ds_Update()
{
	for (int i = 0; i < MAX_OBJ; i++)
	{
		auto obj = &obj_tbl[i];
		if (obj->used && obj->pBuf && obj->stopped == 0)
		{
			if (obj->loops == 0 && obj->str->sample_index == obj->str->loop_end_index)
				obj->Stop();
			else
			{
				if (obj->set_volume)
				{
					obj->SetVolume(obj_tbl[i].volume);
					obj->set_volume = 0;
				}
				obj->SendData();
			}
		}
	}
}

//-------------------------------------
// C++ helpers
void SndObj::CreateBuffer(CriFileStream* stream) { ds_CreateBuffer(this, stream); };
void SndObj::Play() { ds_Play(this); }
void SndObj::Stop() { ds_Stop(this); }

u_long SndObj::GetPosition() { return ds_GetPosition(this); }
int SndObj::GetStatus() { return ds_GetStatus(this); }
void SndObj::SendData() { adxds_SendData(this); }
void SndObj::SetVolume(int vol) { ds_SetVolume(this, vol); }
void SndObj::Release() { ds_Release(this); }
