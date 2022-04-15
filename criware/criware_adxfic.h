/*
*/
#pragma once

typedef struct ADX_Entry
{
	XXH64_hash_t hash;
	std::string filename;
	DWORD size;
} ADX_Entry;

typedef struct ADXFIC_Object
{
	std::vector<ADX_Entry> files;
} ADXFIC_Object;

ADXFIC_Object* adx_ficCreate(const char* dname);
