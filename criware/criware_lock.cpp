/*
* Copyright (C) 2022 Gemini
* ===========================================================
* Thread module
* -----------------------------------------------------------
* Critical section manager for ADX.
* ===========================================================
*/
#include "criware.h"
#include <chrono>

static double GetTime()
{
	return std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() * 1000.;
}

static CRITICAL_SECTION ADX_crit;

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
	double start = GetTime();

	while (!TryEnterCriticalSection(&ADX_crit))
	{
		// deadlock prevention
		double cur = GetTime() - start;
		if (cur > 4000.)	// 4 second threshold to acquire the critical section
		{
			ADXD_Log("Preventing deadlock %3f ms\n", cur);
			break;
		}
	}
}

void ADX_unlock()
{
	LeaveCriticalSection(&ADX_crit);
}
