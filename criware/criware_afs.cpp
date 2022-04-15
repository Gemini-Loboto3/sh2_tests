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
		fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		part_name = filename;
	}

	void Close()
	{
		if (fp != INVALID_HANDLE_VALUE)
		{
			CloseHandle(fp);
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

	DWORD read;
	ReadFile(afs.fp, &head, sizeof(head), &read, nullptr);

	if (head.magic != '\x00SFA' && head.magic != 'AFS\x00')
	{
		afs.Close();
		return 0;
	}

	afs.entries = std::vector<AFS_entry>(head.count);
	ReadFile(afs.fp, afs.entries.data(), sizeof(AFS_entry) * afs.entries.size(), &read, nullptr);
	//CloseHandle(fp);

	afs.part_name = filename;

	return 1;
}

static void cb(LPVOID ctx)
{
	ADXT_Object* obj = (ADXT_Object*)ctx;

	obj->obj->Release();
	obj->obj = nullptr;
	delete obj->stream;
	obj->stream = nullptr;
	obj->initialized = 0;
}

int asf_StartAfs(ADXT_Object* obj, int patid, int fid)
{
	//HANDLE fp = CreateFileA(part_name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	//if (fp == INVALID_HANDLE_VALUE)
	//	return 0;

	ADXStream* stream = new ADXStream;
	stream->Open(afs.fp, afs.entries[fid].pos);
	OpenADX(stream);

	obj->initialized = 1;
	obj->stream = stream;
	obj->obj = ds_FindObj();
	obj->obj->loops = stream->loop_enabled;
	obj->obj->SetEndCallback(cb, obj);

	obj->obj->CreateBuffer(stream);
	obj->obj->Play();

	return 1;
}
