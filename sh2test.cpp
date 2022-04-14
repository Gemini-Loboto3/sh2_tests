// sh2test.cpp : Defines the exported functions for the DLL.
//
#include "framework.h"
#include "sh2test.h"
#include "inject.h"
#include <D3dkmthk.h>
#include <timeapi.h>
#include <mmiscapi2.h>

#include "D3d8.h"
#include "timer.h"
#include "dxrinput/dinput8_proxy.h"
#include "d3d8_proxy.h"

#include "criware\criware.h"

#pragma comment(lib, "Winmm.lib")

void ADXM_WaitVsync();

// makes the exe freely writable on DLL-side of code
void MakePageWritable(unsigned long ulAddress, unsigned long ulSize)
{
	MEMORY_BASIC_INFORMATION* mbi = new MEMORY_BASIC_INFORMATION;
	VirtualQuery((void*)ulAddress, mbi, ulSize);
	if (mbi->Protect != PAGE_EXECUTE_READWRITE)
	{
		unsigned long* ulProtect = new unsigned long;
		VirtualProtect((void*)ulAddress, ulSize, PAGE_EXECUTE_READWRITE, ulProtect);
		delete ulProtect;
	}
	delete mbi;
}

//EXTERN_C DWORD ADXM_WaitVsync();
EXTERN_C int adxm_safe_loop, adxm_safe_cnt, adxm_safe_exit;

#define SYNC_TRICK	ADXM_WaitVsync()

//void __stdcall adxm_safe_proc(LPVOID lpThreadParameter)
//{
//	for (; !adxm_safe_loop; ++adxm_safe_cnt)
//		SYNC_TRICK;
//	adxm_safe_exit = 1;
//	ExitThread(999);
//}

EXTERN_C int adxm_mwidle_loop, adxm_mwidle_cnt, adxm_goto_border_flag, adxm_mwidle_exit, nPriority;
EXTERN_C HANDLE adxm_mwidle_thrdhn;
EXTERN_C int SVM_ExecSvrMwIdle();

//void __stdcall adxm_mwidle_proc(LPVOID lpThreadParameter)
//{
//	while (!adxm_mwidle_loop)
//	{
//		++adxm_mwidle_cnt;
//		if (!SVM_ExecSvrMwIdle() || adxm_goto_border_flag == 1)
//		{
//			if (adxm_goto_border_flag == 1)
//			{
//				adxm_goto_border_flag = 0;
//				SetThreadPriority(adxm_mwidle_thrdhn, nPriority);
//			}
//			//
//			//if (adxm_mwidle_sleep_cb)
//			//    adxm_mwidle_sleep_cb(dword_1D81CFC);
//			SuspendThread(adxm_mwidle_thrdhn);
//		}
//		SYNC_TRICK;
//	}
//	adxm_mwidle_exit = 1;
//	ExitThread(999);
//}

HANDLE WINAPI _CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId)
{
	HANDLE th = CreateThread(lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId);
	if (th)
	{
		switch ((DWORD)lpStartAddress)
		{
		case 0x55FAE0:	// adxm_vsync_proc
		case 0x560190:	// ADXWIN_EnableFsThrd
			SetThreadAffinityMask(th, 1);
		}
	}

	return th;
}

FrameLimiter timer;
EXTERN_C int dword_1D81D18;
EXTERN_C UINT vsync_timer;
EXTERN_C HANDLE hvsync;
HANDLE hvsync_mutex;
int reset_timer = 0;

void ADXM_WaitVsync()
{
	WaitForSingleObject(hvsync_mutex, INFINITE);
	//reset_timer = 0;
	ResetEvent(hvsync_mutex);
}

DWORD WINAPI vsync(LPVOID lpThreadParameter)
{
	while (1)
	{
#if 0
		Sleep(2);
		//int counter = 0;
#else
		while (!timer.Sync());
#endif

		//if(reset_timer == 0)
			SetEvent(hvsync_mutex);
		//reset_timer++;
	}

	return 0;
}

EXTERN_C int adxm_vsync_cnt;

void __stdcall vsync_setup()
{
	timer.Init();
	adxm_vsync_cnt = 0;
	
	hvsync_mutex = CreateEventA(nullptr, FALSE, FALSE, nullptr);
	hvsync = CreateThread(nullptr, 0, vsync, nullptr, 0, nullptr);
	if(hvsync) SetThreadPriority(hvsync, THREAD_PRIORITY_TIME_CRITICAL);
}

EXTERN_C LRESULT __stdcall WndProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
EXTERN_C DInput8Proxy* pDInput;

LRESULT __stdcall WndProcedureEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INPUT:
		pDInput->ProcessRaw(lParam, wParam);
		return 0;
	case WM_INPUT_DEVICE_CHANGE:
		pDInput->UpdateRaw(lParam, wParam);
		return 0;
	}

	return WndProcedure(hWnd, msg, wParam, lParam);
}

void Inject_xaudio2();

EXTERN_C void ADXT_SetupThrd(int *priority);

void ADXT_SetupThrdX(int*)
{
#define PRI		THREAD_PRIORITY_NORMAL

	int p[] =
	{
		THREAD_PRIORITY_IDLE,	// SAVE
		THREAD_PRIORITY_IDLE,	// SAFE
		THREAD_PRIORITY_HIGHEST,	// VSYNC
		PRI,	// unused
		THREAD_PRIORITY_IDLE		// mwidle
	};

	ADXT_SetupThrd(p);
}

EXTERN_C IDirect3DSurface8 *pRenderTarget;

void Inject_tests()
{
	MakePageWritable(0x401000, 0x24A8FFF - 0x401000);

#if 1
	INJECT(0x55F850, ADXFIC_Create);
	INJECT(0x55F890, ADXFIC_GetNumFiles);
	INJECT(0x55F8F0, ADXFIC_GetFileName);

	INJECT(0x55D4C0, AIX_GetInfo);
	INJECT(0x55F740, ADXWIN_SetupDvdFs);
	INJECT(0x55F7F0, ADXWIN_SetupSound);
	INJECT(0x55FD70, ADXM_SetupThrd);
	
	INJECT(0x55E400, ADXF_LoadPartitionNw);
	INJECT(0x55E570, ADXF_GetPtStat);

	INJECT(0x55BE80, ADXT_StartAfs);
	INJECT(0x55BF60, ADXT_StartFname);
	INJECT(0x55C170, ADXT_Create);
	INJECT(0x55C6A0, ADXT_Stop);
	INJECT(0x55C720, ADXT_GetStat);
	INJECT(0x55CC60, ADXT_SetOutVol);
	INJECT(0x5604A0, ADXT_Init);

	INJECT(0x55D540, AIXP_Create);
	INJECT(0x55D740, AIXP_Destroy);
	INJECT(0x55D840, AIXP_StartFname);
	INJECT(0x55D8E0, AIXP_Stop);
	INJECT(0x55D960, AIXP_GetStat);
	INJECT(0x55D970, AIXP_GetAdxt);
	INJECT(0x55D9A0, AIXP_SetLpSw);
	INJECT(0x55DCE0, AIXP_ExecServer);
	
#else
	INJECT_CALL(0x515014, ADXT_SetupThrdX, 5);
	INJECT_CALLX(0x55FDB0, vsync_setup, 0x55FE32);
	INJECT(0x55FFC0, ADXM_WaitVsync);
	memset((void*)0x55FFFC, 0x90, 0x56000B - 0x55FFFC);	// stagger loop in adxm_destroy_thrd
	memset((void*)0x56008C, 0x90, 0x56008F - 0x56008C);	// SetEvent in adxm_destroy_thrd
#endif

	pRenderTarget = pRenderTarget;

	//INJECT_EXT(0x24A66F0, DirectInput8CreateProxy);
	//INJECT(0x4010F0, WndProcedureEx);

	//Inject_xaudio2();
}
