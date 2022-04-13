/*
* =================================================
* ASF partition module
* =================================================
*/
#include "criware.h"
#include <vector>

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

int _ADXF_LoadPartitionNw(int ptid, const char* filename, void* ptinfo, void* nfile)
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

	std::vector<AFS_entry> entries = std::vector<AFS_entry>(head.count);
	ReadFile(fp, entries.data(), sizeof(AFS_entry) * entries.size(), &read, nullptr);

	return 1;
}
