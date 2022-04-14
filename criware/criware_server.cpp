/*
* ===========================================================
* FILE SERVER
* This module uses a thread to fetch data into DirectSound
* buffer for ADX playback and dynamic AIX playback.
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

void server_destroy()
{
	loop = false;
	WaitForSingleObject(hServer, INFINITE);
}
