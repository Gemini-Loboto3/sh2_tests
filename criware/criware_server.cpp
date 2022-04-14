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

enum Command
{
	ADXCMD_LOAD_ADX,
	ADXCMD_LOAD_AIX
};

HANDLE hServer;
static bool loop = true;

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
