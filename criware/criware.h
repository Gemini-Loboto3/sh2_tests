#pragma once
#include <windows.h>
#include <dsound.h>

//-------------------------------------------
class BE16
{
public:
	__inline WORD w() { return (d[0] << 8) | (d[1]); }
	__inline short s() { return (short)w(); }
private:
	BYTE d[2];
};

class BE32
{
public:
	__inline DWORD dw() { return (d[0] << 24) | (d[1] << 16) | (d[2] << 8) | (d[3]); }
	__inline int i() { return (int)dw(); }
private:
	BYTE d[4];
};

#include "criware_file.h"
#include "criware_dsound.h"
#include "criware_aix.h"

//-------------------------------------------
// main exposed module
class ADXT_Object
{
public:
	ADXT_Object() : work_size(0),
		work(nullptr),
		maxch(0),
		stream(nullptr),
		obj(nullptr),
		volume(0)
	{}
	~ADXT_Object()
	{
		if (stream) { delete stream; stream = nullptr; }
		if (obj)
		{
			obj->Stop();
			obj->Release();
			obj = nullptr;
		}
	}

	u_long work_size;
	void* work;
	int maxch;
	CriFileStream* stream;
	SndObj* obj;
	int volume;
};

class AIXP_Object
{
public:
	AIXP_Object() : stream_no(0),
		aix(nullptr)
	{
	}
	~AIXP_Object()
	{
		for (int i = 0; i < stream_no; i++)
		{
			adxt[i].obj->Stop();
			adxt[i].obj->Release();
			adxt[i].obj = nullptr;
		}
		memset(adxt, 0, sizeof(adxt));

		if (aix)
		{
			delete aix;
			aix = nullptr;
		}
	}

	int stream_no;
	AIX_Handle* aix;
	ADXT_Object adxt[8];
};

#define	ADXF_STAT_STOP			(1)			/*	During standstill			*/
#define ADXF_STAT_READING		(2)			/*	During data read-in			*/
#define ADXF_STAT_READEND		(3)			/*	Data read-in end			*/
#define ADXF_STAT_ERROR			(4)			/*	Read-in error outbreak state*/

#define	ADXT_STAT_STOP		(0)		/*	During standstill					*/
#define ADXT_STAT_DECINFO	(1)		/*	Getting header information			*/
#define ADXT_STAT_PREP		(2)		/*	During play preparation				*/
#define ADXT_STAT_PLAYING	(3)		/*	During decode and play				*/
#define ADXT_STAT_DECEND	(4)		/*	Decode end							*/
#define ADXT_STAT_PLAYEND	(5)		/*	Play end							*/
#define ADXT_STAT_ERROR		(6)		/*	Read-in error outbreak state		*/

void  ADXWIN_SetupDvdFs(void* /* ignored */);
void  ADXWIN_SetupSound(LPDIRECTSOUND8 pDS8);
int   ADXM_SetupThrd(int* = nullptr);

void  ADXT_Init();
ADXT_Object* ADXT_Create(int maxch, void* work, u_long work_size);
void  ADXT_Stop(ADXT_Object* obj);
int   ADXT_GetStat(ADXT_Object* obj);
void  ADXT_StartFname(ADXT_Object* obj, const char* fname);
void  ADXT_SetOutVol(ADXT_Object* obj, int);
void  ADXT_StartAfs(ADXT_Object* obj, int patid, int fid);

void  AIX_GetInfo();
int   ADXF_GetPtStat(int);

void* ADXFIC_Create(const char* dname, int mode, char* work, int wksize);
u_long ADXFIC_GetNumFiles(void* obj);
const char* ADXFIC_GetFileName(void* obj, u_long index);

int   ADXF_LoadPartitionNw(int ptid, const char* filename, void* ptinfo, void* nfile);

int   asf_LoadPartitionNw(int ptid, const char* filename, void* ptinfo, void* nfile);
int   asf_StartAfs(ADXT_Object* obj, int patid, int fid);

void AIXP_Stop(AIXP_Object *obj);
void AIXP_ExecServer();
void AIXP_Destroy(AIXP_Object *obj);
AIXP_Object* AIXP_Create(int maxntr, int maxnch, void* work, int worksize);
void AIXP_SetLpSw(AIXP_Object *obj, int sw);
void AIXP_StartFname(AIXP_Object *obj, const char *fname, void *atr);
ADXT_Object* AIXP_GetAdxt(AIXP_Object *obj, int trno);
int AIXP_GetStat(AIXP_Object *obj);

//-------------------------------------------
// file server module
extern HANDLE hServer;

DWORD WINAPI server_thread(LPVOID params);

//-------------------------------------------
typedef struct ADX_headerV3
{
	BE16 magic;					// 0
	BE16 copyright_offset;		// 2
	BYTE encoding,				// 4
		block_size,				// 5
		sample_bitdepth,		// 6
		channel_count;			// 7
	BE32 sample_rate,			// 8
		total_samples;			// C
	BE16 highpass_frequency;	// 10
	BYTE version,				// 12
		flags;					// 13
	BE16 loop_align_samples,	// 14
		loop_enabled;			// 16
	BE32 loop_enabled2,			// 18
		loop_sample_begin,		// 1C
		loop_byte_begin,		// 20
		loop_sample_end,		// 24
		loop_byte_end,			// 28
		dummy0,					// 2C
		dummy1,					// 30
		dummy2,					// 34
		dummy3,					// 38
		dummy4;					// 3C
} ADX_headerV3;

typedef struct ADX_headerAIX
{
	BE16 magic;					// 0
	BE16 copyright_offset;		// 2
	BYTE encoding,				// 4
		block_size,				// 5
		sample_bitdepth,		// 6
		channel_count;			// 7
	BE32 sample_rate,			// 8
		total_samples;			// C
	BE16 highpass_frequency;	// 10
	BYTE version,				// 12
		flags;					// 13
} ADX_header_AIX;

typedef struct ADX_headerV4
{
	BE16 magic;					// 0
	BE16 copyright_offset;		// 2
	BYTE encoding,				// 4
		block_size,				// 5
		sample_bitdepth,		// 6
		channel_count;			// 7
	BE32 sample_rate,			// 8
		total_samples;			// C
	BE16 highpass_frequency;	// 10
	BYTE version,				// 12
		flags;					// 13
	BE16 loop_align_samples,	// 14
		dummy0;					// 16
	BE32 dummy1,				// 18
		dummy2,					// 1C
		dummy3,					// 20
		loop_enabled,			// 24
		loop_sample_begin,		// 28
		loop_byte_begin,		// 2C
		loop_sample_end,		// 30
		loop_byte_end,			// 34
		dummy4,					// 38
		dummy5;					// 3C
} ADX_headerV4;

typedef struct ADX_Handle
{
	HANDLE fp;
	//
	u_long copyright_offset,
		block_size,
		sample_bitdepth,
		channel_count,
		sample_rate,
		total_samples,
		highpass_frequency,
		loop_enabled,
		loop_start_index,
		loop_end_index;
	int* past_samples;		// Previously decoded samples from each channel, zeroed at start (size = 2*channel_count)
	u_long sample_index;		// sample_index is the index of sample set that needs to be decoded next
	double coefficient[2];
} ADX_HANDLE;

typedef struct ADX {} ADX;

void adx_set_coeff(CriFileStream* adx);
unsigned decode_adx_standard(CriFileStream* adx, short* buffer, unsigned samples_needed, bool looping_enabled);

int OpenADX(const char* filename, ADXStream** obj);
int OpenADX(ADXStream* adx);
void CloseADX(ADX* obj);
