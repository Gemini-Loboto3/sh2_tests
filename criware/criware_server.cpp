/*
* Copyright (C) 2022 Gemini
* ===========================================================
* File server module
* -----------------------------------------------------------
* This module uses a thread to fetch data to DirectSound
* buffers for ADX and AIX playback.
* ===========================================================
*/
#include "criware.h"

static HANDLE hServer;
static bool loop = true;
static CRITICAL_SECTION ADX_crit;

//
void ADX_lock_init()
{
	InitializeCriticalSection(&ADX_crit);
}

void ADX_lock_close()
{
	DeleteCriticalSection(&ADX_crit);
}

void ADX_lock()
{
	EnterCriticalSection(&ADX_crit);
}

void ADX_unlock()
{
	LeaveCriticalSection(&ADX_crit);
}

DWORD WINAPI server_thread(LPVOID params)
{
	while (loop)
	{
		ds_Update();
		Sleep(10);
	}

	return 0;
}

void server_create()
{
	hServer = CreateThread(nullptr, 0, server_thread, nullptr, CREATE_SUSPENDED, nullptr);
	if (hServer)
	{
		SetThreadPriority(hServer, THREAD_PRIORITY_ABOVE_NORMAL);	// make sure the thread is snappy
		ResumeThread(hServer);
	}
}

void server_destroy()
{
	loop = false;
	WaitForSingleObject(hServer, INFINITE);
}
