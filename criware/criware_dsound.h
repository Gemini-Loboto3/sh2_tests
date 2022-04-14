#pragma once

typedef struct SndObj
{
	u_long used,
		offset;
	WAVEFORMATEX fmt;
	LPDIRECTSOUNDBUFFER pBuf;
	CriFileStream* str;
} SndObj;

void adxds_SetupSound(LPDIRECTSOUND8 pDS);
void adxds_Update();

SndObj* adxds_FindObj();
void adxds_CreateBuffer(SndObj* obj, CriFileStream* stream);
u_long adxds_GetPosition(SndObj* obj);
void adxds_SendData(SndObj* obj);
void adxds_SetVolume(SndObj* obj, int vol);
void adxds_Play(SndObj* obj);
void adxds_Stop(SndObj* obj);
int adxds_GetStatus(SndObj* obj);
