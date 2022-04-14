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

ADX_Dir* ADXFIC_Create(const char* dname, int mode, char *work, int wksize)
{
	ADX_Dir *dir = new ADX_Dir;
	// list all the files
	list_data_folder(dname, *dir);
	// sort entries by hashes for binary find
	std::sort(dir->files.begin(), dir->files.end(), [](ADX_Entry& a, ADX_Entry& b) { return a.hash < b.hash; });

	return dir;
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

// destroys the ADX threads
void ADXM_DestroyThrd()
{
	server_destroy();
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
