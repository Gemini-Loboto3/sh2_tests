#include "framework.h"
#include <stdlib.h>
#include "timer.h"

void FrameLimiter::Init()
{
	LARGE_INTEGER frequency;

	QueryPerformanceFrequency(&frequency);
	TIME_Frequency = (double)frequency.QuadPart / (double)TICKS_PER_SECOND;
	Ticks();
}

void FrameLimiter::Ticks()
{
	LARGE_INTEGER counter;

	QueryPerformanceCounter(&counter);
	TIME_Ticks = (double)counter.QuadPart / TIME_Frequency;
}

DWORD FrameLimiter::Sync()
{
	DWORD lastTicks, currentTicks;
	LARGE_INTEGER counter;

	QueryPerformanceCounter(&counter);
	lastTicks = (DWORD)TIME_Ticks;
	TIME_Ticks = (double)counter.QuadPart / TIME_Frequency;
	currentTicks = (DWORD)TIME_Ticks;

	return (currentTicks > lastTicks) ? currentTicks - lastTicks : 0;
}

