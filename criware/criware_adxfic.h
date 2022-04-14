/*
*/
#pragma once

typedef struct ADX_Entry
{
	XXH64_hash_t hash;
	std::string filename;
	DWORD size;
} ADX_Entry;

typedef struct ADX_Dir
{
	std::vector<ADX_Entry> files;
} ADX_Dir;

ADX_Dir* adx_ficCreate(const char* dname);
