/*
* Copyright (C) 2022 Gemini
* ===============================================================
* AFS partition module
* ---------------------------------------------------------------
* Nothing too fancy, it's just an archive with offsets and sizes
* in the header. It also stores file names, but they are ignored
* as the access is managed via IDs.
* ===============================================================
*/
#include "criware.h"

typedef struct AFS_header
{
	u_long magic;	// "AFS\x00"
	u_long count;
} AFS_header;

typedef struct AFS_entry
{
	u_long pos,
		size;
} AFS_entry;

class AFS_Object
{
public:
	AFS_Object() : fp(INVALID_HANDLE_VALUE)
	{}
	~AFS_Object()
	{
		Close();
	}

	void Open(const char* filename)
	{
		fp = ADXF_OpenFile(filename);
		part_name = filename;
	}

	void Close()
	{
		if (fp != INVALID_HANDLE_VALUE)
		{
			ADXF_CloseFile(fp);
			fp = INVALID_HANDLE_VALUE;
		}
	}

	std::vector<AFS_entry> entries;
	HANDLE fp;
	std::string part_name;
};

AFS_Object afs;

int asf_LoadPartitionNw(int ptid, const char* filename, void* ptinfo, void* nfile)
{
	afs.Open(filename);
	if (afs.fp == INVALID_HANDLE_VALUE)
		return 0;

	AFS_header head;

	ADXF_ReadFile(afs.fp, &head, sizeof(head));

	if (head.magic != '\x00SFA' && head.magic != 'AFS\x00')
	{
		afs.Close();
		return 0;
	}

	afs.entries = std::vector<AFS_entry>(head.count);
	ADXF_ReadFile(afs.fp, afs.entries.data(), sizeof(AFS_entry) * afs.entries.size());

	afs.part_name = filename;

	return 1;
}

static void cb(LPVOID ctx)
{
	ADXT_Object* obj = (ADXT_Object*)ctx;
	//ADXT_Stop(obj);
	obj->state = ADXT_STAT_PLAYEND;
	obj->obj->stopped = 1;
}

int asf_StartAfs(ADXT_Object* obj, int patid, int fid)
{
	CriFileStream* stream;
	
	// read magic word to detect type
	ADXF_Seek(afs.fp, afs.entries[fid].pos, FILE_BEGIN);
	DWORD magic;
	ADXF_ReadFile(afs.fp, &magic, sizeof(magic));

	// detect RIFF wave
	if (magic == 'RIFF' || magic == 'FFIR')
	{
		auto wav = new WAVStream;
		// we need to make a new handle for wav or this will crash badly
		HANDLE fp = ADXF_OpenFile(afs.part_name.c_str());
		ADXF_Seek(fp, afs.entries[fid].pos, FILE_BEGIN);
		if (wav->Open(fp, afs.entries[fid].pos) == S_FALSE)
		{
			MessageBoxA(nullptr, "Error opening WAV stream.", __FUNCTION__, MB_ICONERROR);
			return 0;
		}
		stream = wav;
	}
	// assume ADX
	else
	{
		// rewind back to where we need to be
		ADXF_Seek(afs.fp, afs.entries[fid].pos, FILE_BEGIN);

		auto adx = new ADXStream;
		adx->Open(afs.fp, afs.entries[fid].pos);
		if (FAILED(OpenADX(adx)))
		{
			MessageBoxA(nullptr, "Error opening ADX stream.", __FUNCTION__, MB_ICONERROR);
			return 0;
		}
		stream = adx;
	}

	obj->state = ADXT_STAT_PLAYING;
	obj->stream = stream;
	obj->obj = ds_FindObj();
	obj->obj->loops = stream->loop_enabled;
	obj->obj->SetEndCallback(cb, obj);

	obj->obj->CreateBuffer(stream);
	obj->obj->Play();

	return 1;
}
