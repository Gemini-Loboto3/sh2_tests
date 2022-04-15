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

static std::vector<AFS_entry> entries;
static HANDLE fp;
static std::string part_name;

int asf_LoadPartitionNw(int ptid, const char* filename, void* ptinfo, void* nfile)
{
	HANDLE fp = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fp == INVALID_HANDLE_VALUE)
		return 0;

	AFS_header head;

	DWORD read;
	ReadFile(fp, &head, sizeof(head), &read, nullptr);

	if (head.magic != '\x00SFA' && head.magic != 'AFS\x00')
	{
		CloseHandle(fp);
		return 0;
	}

	entries = std::vector<AFS_entry>(head.count);
	ReadFile(fp, entries.data(), sizeof(AFS_entry) * entries.size(), &read, nullptr);
	CloseHandle(fp);

	part_name = filename;

	return 1;
}

int asf_StartAfs(ADXT_Object* obj, int patid, int fid)
{
	HANDLE fp = CreateFileA(part_name.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fp == INVALID_HANDLE_VALUE)
		return 0;

	SetFilePointer(fp, entries[fid].pos, nullptr, FILE_BEGIN);

	ADXStream* stream = new ADXStream;
	stream->Open(fp);
	OpenADX(stream);

	obj->initialized = 1;
	obj->stream = stream;
	obj->obj = ds_FindObj();
	obj->obj->loops = false;

	obj->obj->CreateBuffer(stream);
	obj->obj->Play();

	return 1;
}
