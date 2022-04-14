/*
* ===============================================================
* DirectSound interface for buffers
* ===============================================================
*/
#include <dsound.h>
#include "criware.h"

LPDIRECTSOUND8 pDS8;

#define MAX_OBJ			32
#define BUFFER_SIZE		32768
#define BUFFER_HALF		(BUFFER_SIZE / 2)
#define BUFFER_QUART	(BUFFER_HALF / 2)

static SndObj obj_tbl[MAX_OBJ];

void adxds_SetupSound(LPDIRECTSOUND8 pDS)
{
	pDS8 = pDS;

	for (int i = 0; i < MAX_OBJ; i++)
		obj_tbl[i].used = 0;
}

SndObj* adxds_FindObj()
{
	for (int i = 0; MAX_OBJ; i++)
	{
		if (obj_tbl[i].used == 0)
			return &obj_tbl[i];
	}

	return nullptr;
}

void adxds_CreateBuffer(SndObj *obj, CriFileStream* stream)
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
	desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	desc.dwBufferBytes = BUFFER_SIZE;
	if (FAILED(pDS8->CreateSoundBuffer(&desc, &obj->pBuf, nullptr)))
		MessageBoxA(nullptr, "error", __FUNCTION__, MB_OK);

	DWORD bytes1, bytes2;
	short* ptr1, * ptr2;
	obj->pBuf->Lock(0, BUFFER_SIZE, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
	decode_adx_standard(stream, ptr1, bytes1 / obj->fmt.nBlockAlign, 1);
	obj->pBuf->Unlock(ptr1, bytes1, ptr2, bytes2);

	obj->used = 1;
	obj->offset = 0;
}

u_long adxds_GetPosition(SndObj* obj)
{
	DWORD pos;
	obj->pBuf->GetCurrentPosition(&pos, nullptr);

	return pos;
}

void adxds_SendData(SndObj *obj)
{
	u_long add = 0, pos;
	const DWORD snd_dwBytes = BUFFER_QUART;

	adxds_GetPosition(obj);
	obj->pBuf->GetCurrentPosition(&pos, nullptr);

	if (pos - obj->offset < 0)
		add = BUFFER_SIZE;
	if (pos + add - obj->offset > 2 * snd_dwBytes + 16)
	{
		DWORD bytes1, bytes2;
		short* ptr1, * ptr2;

		obj->pBuf->Lock(obj->offset, snd_dwBytes, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
		decode_adx_standard(obj->str, ptr1, bytes1 / obj->fmt.nBlockAlign, obj->loops);
		obj->pBuf->Unlock(ptr1, bytes1, ptr2, bytes2);

		auto total = snd_dwBytes + obj->offset;
		obj->offset = total;
		if (BUFFER_SIZE <= total)
			obj->offset = total - BUFFER_SIZE;
	}
}

void adxds_SetVolume(SndObj* obj, int vol)
{
	obj->volume = vol;
	if (obj->used && obj->pBuf)
		obj->pBuf->SetVolume(vol * 10);
}

void adxds_Play(SndObj* obj)
{
	if (obj->used && obj->pBuf)
		obj->pBuf->Play(0, 0, DSBPLAY_LOOPING);
}

void adxds_Stop(SndObj* obj)
{
	if (obj->used && obj->pBuf)
	{
		obj->pBuf->Stop();
		while (adxds_GetStatus(obj) == ADXT_STAT_PLAYING);
	}
}

int adxds_GetStatus(SndObj* obj)
{
	if (obj->used == 0)
		return ADXT_STAT_STOP;

	DWORD status;
	obj->pBuf->GetStatus(&status);

	if(status == DSBSTATUS_LOOPING || status == DSBSTATUS_PLAYING)
		return ADXT_STAT_PLAYING;

	return ADXT_STAT_PLAYEND;
}

void adxds_Release(SndObj* obj)
{
	if (obj->used)
	{
		obj->pBuf->Release();
		memset(obj, 0, sizeof(*obj));
	}
}

void adxds_Update()
{
	for (int i = 0; i < MAX_OBJ; i++)
	{
		if (obj_tbl[i].used)
		{
			if (obj_tbl[i].loops == 0 && obj_tbl[i].str->sample_index == obj_tbl[i].str->loop_end_index)
				obj_tbl[i].Stop();
			else obj_tbl[i].SendData();
		}
	}
}

//-------------------------------------
void SndObj::CreateBuffer(CriFileStream* stream) { adxds_CreateBuffer(this, stream); };
void SndObj::Play() { adxds_Play(this); }
void SndObj::Stop() { adxds_Stop(this); }

u_long SndObj::GetPosition() { return adxds_GetPosition(this); }
int SndObj::GetStatus() { return adxds_GetStatus(this); }
void SndObj::SendData() { adxds_SendData(this); }
void SndObj::SetVolume(int vol) { adxds_SetVolume(this, vol); }
void SndObj::Release() { adxds_Release(this); }
