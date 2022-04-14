#pragma once

class SndObj
{
public:
	SndObj() : used(0),
		offset(0),
		loops(0),
		volume(0),
		fmt {0},
		pBuf(nullptr),
		str(nullptr)
	{}
	~SndObj()
	{}

	u_long used,
		offset,
		loops;
	int volume;
	WAVEFORMATEX fmt;
	LPDIRECTSOUNDBUFFER pBuf;
	CriFileStream* str;
};

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
void adxds_Release(SndObj* obj);
