/*
* Copyright (C) 2022 Gemini
* ===============================================================
* Debugging module
* ---------------------------------------------------------------
* Generic interface for handling error messages.
* ===============================================================
*/

#include "criware.h"

static int dlevel = 0;

void ADXD_Error(const char* caption, const char* fmt, ...)
{
	if (dlevel)
	{
		va_list ap;
		char buf[256];

		va_start(ap, fmt);
		vsprintf_s(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		MessageBoxA(nullptr, buf, caption, MB_ICONERROR | MB_DEFAULT_DESKTOP_ONLY);
		exit(0);
	}
}

void ADXD_Warning(const char* caption, const char* fmt, ...)
{
	if (dlevel)
	{
		va_list ap;
		char buf[256];

		va_start(ap, fmt);
		vsprintf_s(buf, sizeof(buf), fmt, ap);
		va_end(ap);

		MessageBoxA(nullptr, buf, caption, MB_ICONEXCLAMATION | MB_DEFAULT_DESKTOP_ONLY);
	}
}

void ADXD_Log(const char* fmt, ...)
{
	if (dlevel)
	{
		va_list ap;
		char buf[256];

		va_start(ap, fmt);
		vsprintf_s(buf, fmt, ap);
		va_end(ap);

		OutputDebugStringA(buf);
	}
}

void ADXD_SetLevel(int level)
{
	dlevel = level < 2 ? 0 : 1;
}
