#pragma once

enum DSOBJ_STATE
{
	DSOS_UNUSED,
	DSOS_PLAYING,
	DSOS_LOOPING,
	DSOS_ENDED
};

class SndObj
{
public:
	SndObj() : used(0),
		offset(0),
		loops(0),
		volume(0),
		set_volume(0),
		stopped(0),
		trans_lock(0),
		fmt {0},
		pBuf(nullptr),
		str(nullptr)
	{}
	~SndObj()
	{}

	void CreateBuffer(CriFileStream* stream);

	void Play();
	void Stop();

	u_long GetPosition();
	int GetStatus();
	void SendData();
	void SetVolume(int vol);
	void Release();

	u_long offset;
	u_long used : 1,
		loops : 1,
		stopped : 1,
		set_volume : 1,
		trans_lock : 1;		// failsafe for locking release when it's transferring
	int volume;
	WAVEFORMATEX fmt;
	LPDIRECTSOUNDBUFFER pBuf;
	CriFileStream* str;
};

void ds_SetupSound(LPDIRECTSOUND8 pDS);
void ds_Update();

SndObj* ds_FindObj();

void ds_CreateBuffer(SndObj* obj, CriFileStream* stream);
u_long ds_GetPosition(SndObj* obj);
void adxds_SendData(SndObj* obj);
void ds_SetVolume(SndObj* obj, int vol);
void ds_Play(SndObj* obj);
void ds_Stop(SndObj* obj);
int ds_GetStatus(SndObj* obj);
void ds_Release(SndObj* obj);
