/*
* Copyright (C) 2022 Gemini
* ===========================================================
* Thread module
* -----------------------------------------------------------
* Critical section manager for ADX.
* ===========================================================
*/
#include "criware.h"

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
	while (!TryEnterCriticalSection(&ADX_crit));
}

void ADX_unlock()
{
	LeaveCriticalSection(&ADX_crit);
}
