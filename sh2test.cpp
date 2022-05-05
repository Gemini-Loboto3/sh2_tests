// sh2test.cpp : Defines the exported functions for the DLL.
//
#include "framework.h"
#include "inject.h"
#include "dxrinput/dinput8_proxy.h"
#include "criware\criware.h"
#include <chrono>
#include "daudio2/daudio.h"

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
	//switch (msg)
	//{
	////case WM_INPUT:
	////	pDInput->ProcessRaw(lParam, wParam);
	////	return 0;
	////case WM_INPUT_DEVICE_CHANGE:
	////	pDInput->UpdateRaw(lParam, wParam);
	////	return 0;
	//}

	return WndProcedure(hWnd, msg, wParam, lParam);
}

void Inject_xaudio2();

typedef struct CUTSCENE_DATA
{
	u_short timer0,
		timer1;
} CUTSCENE_DATA;

void PatchCriware()
{
	// might be needed to catch any uncaught ADX code
	memset((void*)0x55BE80, 0xFB, 0x56FB90 - 0x55BE80);

	INJECT(0x55F850, ADXFIC_Create);
	INJECT(0x55F890, ADXFIC_GetNumFiles);
	INJECT(0x55F8F0, ADXFIC_GetFileName);
	INJECT(0x55F860, ADXFIC_Destroy);

	INJECT(0x55F740, ADXWIN_SetupDvdFs);
	INJECT(0x55F7C0, ADXWIN_ShutdownDvdFs);
	INJECT(0x55F7F0, ADXWIN_SetupSound);
	INJECT(0x55F800, ADXWIN_ShutdownSound);

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

	ADXD_SetLevel(1);
}

void Inject_tests()
{
	MakePageWritable(0x401000, 0x24A8FFF - 0x401000);

	*(BYTE*)0x449019 = 0;		// fix that goddamn stupid lag

	// disable safe mode
	*(BYTE*)0x4F7510 = 0xC3;
	*(BYTE*)0x4F7010 = 0xC3;

#if _DEBUG
	*(BYTE*)0x45A3F0 = 0xC3;	// kill mouse support, release mouse when debugging
#endif

	PatchCriware();

	auto cut = (CUTSCENE_DATA*)0x8DAEEC;
	cut[1].timer1 = 181;

#if XAUDIO2
	INJECT_EXT(0x24A6720, DirectAudioCreate);
	Inject_xaudio2();
#endif

	//INJECT_EXT(0x24A66F0, DirectInput8CreateProxy);
	//INJECT(0x4010F0, WndProcedureEx);
}
