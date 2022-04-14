/*
* Copyright (C) 2022 Gemini
* ===============================================================
* Main ADX interface
* ---------------------------------------------------------------
* Exposes the original functions used in Criware's ADX library,
* helpful for hooking to the game's code.
* ===============================================================
*/
#include "criware.h"

ADX_Dir* ADXFIC_Create(const char* dname, int mode, char *work, int wksize)
{
	return adx_ficCreate(dname);
}

void ADXFIC_Destroy(ADX_Dir* obj)
{
	delete obj;
}

u_long ADXFIC_GetNumFiles(ADX_Dir* obj)
{
	return obj->files.size();
}

const char* ADXFIC_GetFileName(ADX_Dir* obj, u_long index)
{
	return obj->files[index].filename.c_str();
}

// no need to fill these, it's just some leftovers from the XBOX library
void ADXWIN_SetupDvdFs(void*) {}
void ADXWIN_ShutdownDvdFs() {}

void ADXWIN_SetupSound(LPDIRECTSOUND8 pDS8)
{
	ds_SetupSound(pDS8);
}

// initializes the threads used by the ADX server
// in our case it just creates one thread running in parallel with the game
void ADXM_SetupThrd(int* priority /* ignored */)
{
	server_create();
}

// destroys the ADX threads
void ADXM_ShutdownThrd()
{
	server_destroy();
}

int ADXF_LoadPartitionNw(int ptid, const char *filename, void *ptinfo, void *nfile)
{
	asf_LoadPartitionNw(ptid, filename, ptinfo, nfile);

	return 1;
}

// AFS partition read status, just pretent it's done
int ADXF_GetPtStat(int)
{
	return ADXF_STAT_READEND;
}

// returns an ADXT_STAT value
int ADXT_GetStat(ADXT_Object* obj)
{
	if (obj->initialized == 0)
		return ADXT_STAT_STOP;

	switch (obj->obj->GetStatus())
	{
	case DSOS_LOOPING:
	case DSOS_PLAYING:
		return ADXT_STAT_PLAYING;
	case DSOS_UNUSED:
		return ADXT_STAT_STOP;
	case DSOS_ENDED:
		return ADXT_STAT_PLAYEND;
	}

	return ADXT_STAT_STOP;
}

void ADXT_SetOutVol(ADXT_Object *obj, int volume)
{
	obj->volume;
}

// Starts to play ADX file with partition ID and file ID
void ADXT_StartAfs(ADXT_Object* obj, int patid, int fid)
{
	asf_StartAfs(obj, patid, fid);
}

void ADXT_Stop(ADXT_Object* obj)
{
	if (obj->obj)
	{
		ds_Stop(obj->obj);
		ds_Release(obj->obj);
		obj->obj = nullptr;
		obj->stream = nullptr;
	}
}

void ADXT_StartFname(ADXT_Object* obj, const char* fname)
{
	ADXStream *stream;
	OpenADX(fname, &stream);

	obj->initialized = 1;
	obj->stream = stream;
	obj->obj = ds_FindObj();

	obj->obj->CreateBuffer(stream);
	obj->obj->loops = true;
	obj->obj->Play();
}

// initializes the "talk" module, whatever that does
void ADXT_Init()
{}

// theoretically destroys all active talk handles
void ADXT_Finish()
{}

// Creation of an ADXT handle
ADXT_Object* ADXT_Create(int maxch, void* work, u_long work_size)
{
	ADXT_Object* obj = new ADXT_Object;
	obj->maxch = maxch;
	obj->work = work;
	obj->work_size = work_size;

	return obj;
}

// Destroy an ADXT handle
void ADXT_Destroy(ADXT_Object* adxt)
{
	delete adxt;
}

void AIX_GetInfo()
{}

// leave empty
void AIXP_ExecServer() {}

AIXP_Object* AIXP_Create(int maxntr, int maxnch, void* work, int worksize)
{
	AIXP_Object* obj = new AIXP_Object;

	for (int i = 0; i < _countof(obj->adxt); i++)
	{
		obj->adxt[i].maxch = maxnch;
		obj->adxt[i].work_size = worksize;
		obj->adxt[i].work = work;
	}

	return obj;
}

void AIXP_Destroy(AIXP_Object* obj)
{
	delete obj;
}

void AIXP_Stop(AIXP_Object* obj)
{
	for (int i = 0; i < obj->stream_no; i++)
	{
		obj->adxt[i].obj->Stop();
		obj->adxt[i].obj->Release();
	}
}

// set if this should loop
void AIXP_SetLpSw(AIXP_Object* obj, int sw)
{
}

void AIXP_StartFname(AIXP_Object* obj, const char* fname, void* atr)
{
	AIX_Handle* aix;

	OpenAIX(fname, &aix);
	obj->aix = aix;
	obj->stream_no = aix->parent->stream_count;

	for (int i = 0; i < obj->stream_no; i++)
	{
		obj->adxt[i].stream = &aix->parent->stream[i];
		obj->adxt[i].obj = ds_FindObj();
		obj->adxt[i].obj->CreateBuffer(obj->adxt[i].stream);
		obj->adxt[i].obj->loops = true;
	}

	for (int i = 0; i < obj->stream_no; i++)
	{
		obj->adxt[i].initialized = 1;
		obj->adxt[i].obj->Play();
	}

	obj->initialized = 1;
}

ADXT_Object* AIXP_GetAdxt(AIXP_Object* obj, int trno)
{
	if (trno >= obj->stream_no)
		return nullptr;

	return &obj->adxt[trno];
}

int AIXP_GetStat(AIXP_Object* obj)
{
	if (obj->initialized == 0)
		return AIXP_STAT_STOP;

	switch (obj->adxt[0].obj->GetStatus())
	{
	case DSOS_PLAYING:
	case DSOS_LOOPING:
		return AIXP_STAT_PLAYEND;
	case DSOS_UNUSED:
		return AIXP_STAT_STOP;
	case DSOS_ENDED:
		return AIXP_STAT_PLAYEND;
	}

	return AIXP_STAT_STOP;
}
