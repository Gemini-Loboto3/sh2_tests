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

EXTERN_C LRESULT __stdcall WndProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
EXTERN_C DInput8Proxy* pDInput;

LRESULT __stdcall WndProcedureEx(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SETCURSOR:
		ShowCursor(true);
		break;
	//case WM_INPUT:
	//	pDInput->ProcessRaw(lParam, wParam);
	//	return 0;
	//case WM_INPUT_DEVICE_CHANGE:
	//	pDInput->UpdateRaw(lParam, wParam);
	//	return 0;
	}

	return WndProcedure(hWnd, msg, wParam, lParam);
}

void Inject_xaudio2();

void Inject_tests()
{
	MakePageWritable(0x401000, 0x24A8FFF - 0x401000);

#if 1
	//memset((void*)0x408A4A, 0x90, 5);	// remove Performance_set_thread

	INJECT(0x55F850, ADXFIC_Create);
	INJECT(0x55F890, ADXFIC_GetNumFiles);
	INJECT(0x55F8F0, ADXFIC_GetFileName);
	INJECT(0x55F860, ADXFIC_Destroy);

	INJECT(0x55F740, ADXWIN_SetupDvdFs);
	INJECT(0x55F7C0, ADXWIN_ShutdownDvdFs);
	INJECT(0x55F7F0, ADXWIN_SetupSound);

	INJECT(0x55FD70, ADXM_SetupThrd);
	INJECT(0x55FFB0, ADXM_ExecMain);
	INJECT(0x55FFD0, ADXM_ShutdownThrd);
	
	INJECT(0x55E400, ADXF_LoadPartitionNw);
	INJECT(0x55E570, ADXF_GetPtStat);

	INJECT(0x55BE80, ADXT_StartAfs);
	INJECT(0x55BF60, ADXT_StartFname);
	INJECT(0x55C170, ADXT_Create);
	INJECT(0x55C6A0, ADXT_Stop);
	INJECT(0x55C720, ADXT_GetStat);
	INJECT(0x55CC60, ADXT_SetOutVol);
	INJECT(0x5604A0, ADXT_Init);
	INJECT(0x560580, ADXT_Finish);

	INJECT(0x55D4C0, AIXP_Init);
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

	//INJECT_EXT(0x24A66F0, DirectInput8CreateProxy);
	//INJECT(0x4010F0, WndProcedureEx);

	//Inject_xaudio2();
}
