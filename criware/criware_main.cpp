/*
* =================================================
* Public interface
* Exposes the original functions used in Criware,
* necessary for hooking into the game.
* =================================================
*/

#include <windows.h>
#include <string>
#include <vector>
#include <algorithm>
#include <dsound.h>
#include "criware.h"
#include "..\xxhash.h"

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

static void list_data_folder(const char *path, ADX_Dir &dir)
{
	WIN32_FIND_DATAA data;

	char filter[MAX_PATH];
	sprintf_s(filter, sizeof(filter), "%s\\*.*", path);

	HANDLE hFind = FindFirstFileA(filter, &data);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			// ignore this folder and previous folder values
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && data.cFileName[0] == '.')
				continue;

			// full file path
			sprintf_s(filter, "%s\\%s", path, data.cFileName);

			// it's a folder, go one level deeper
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				list_data_folder(filter, dir);
			// it's a regular file, add to queue
			else
			{
				HANDLE fp = CreateFileA(filter, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
				if (fp != INVALID_HANDLE_VALUE)
				{
					ADX_Entry entry;
					entry.filename = filter;
					entry.size = GetFileSize(fp, nullptr);
					entry.hash = XXH64(entry.filename.c_str(), entry.filename.size(), 0);
					CloseHandle(fp);
					dir.files.push_back(entry);
				}
			}

		} while (FindNextFileA(hFind, &data));
		FindClose(hFind);
	}
}

int quickfind(ADX_Entry* f, size_t size, XXH64_hash_t hash)
{
	int first = 0,
		last = (int)size - 1,
		middle = last / 2;

	while (first <= last)
	{
		if (f[middle].hash < hash)
			first = middle + 1;
		else if (f[middle].hash == hash)
			return middle;
		else last = middle - 1;

		middle = (first + last) / 2;
	}

	return -1;
}

void* ADXFIC_Create(const char* dname, int mode, char *work, int wksize)
{
	ADX_Dir *dir = new ADX_Dir;
	// list all the files
	list_data_folder(dname, *dir);
	// sort entries by hashes for binary find
	std::sort(dir->files.begin(), dir->files.end(), [](ADX_Entry& a, ADX_Entry& b) { return a.hash < b.hash; });

	return dir;
}

void ADXFIC_Destroy(void* obj)
{
	delete ((ADX_Dir*)obj);
}

u_long ADXFIC_GetNumFiles(void *obj)
{
	return ((ADX_Dir*)obj)->files.size();
}

const char* ADXFIC_GetFileName(void* obj, u_long index)
{
	return ((ADX_Dir*)obj)->files[index].filename.c_str();
}

// no need to fill this, it's just some leftover from the XBOX library
void ADXXB_SetupDvdFs(void* /* ignored */)
{
}

// sets the internal object system to use a DirectSound8 interface
// fill later with all the necessary methods
void ADXXB_SetupSound(LPDIRECTSOUND8 pDS8)
{

}

// initializes the threads used by the ADX server
// in our case it just creates one thread running in parallel with the game
int ADXM_SetupThrd(int* priority /* ignored */)
{
	hServer = CreateThread(nullptr, 0, server_thread, nullptr, CREATE_SUSPENDED, nullptr);
	if (hServer)
	{
		SetThreadPriority(hServer, THREAD_PRIORITY_ABOVE_NORMAL);
		ResumeThread(hServer);
	}

	return 1;
}

int ADXF_LoadPartitionNw(int ptid, const char *filename, void *ptinfo, void *nfile)
{
	_ADXF_LoadPartitionNw(ptid, filename, ptinfo, nfile);

	return 1;
}

// AFS partition read status, just pretent it's done
int ADXF_GetPtStat(int)
{
	return ADXF_STAT_READEND;
}

// returns an ADXT_STAT value
int ADXT_GetStat(void*)
{
	return ADXT_STAT_PLAYING;
}

void ADXT_SetOutVol(int, int)
{

}

int ADXT_StartAfs(int, int, int)
{
	return 0;
}