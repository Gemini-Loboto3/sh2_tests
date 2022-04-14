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

DWORD WINAPI server_thread(LPVOID params)
{
	while (1)
	{
		adxds_Update();

		Sleep(10);
	}
}
