// dllmain.cpp : Defines the entry point for the DLL application.
#include "framework.h"
#include <dsound.h>
#include "inject.h"
#include "criware/criware.h"
#include <vector>

void SetD3D8();
void ClearD3D8();

void Inject_tests();

#ifdef _USRDLL
BOOL APIENTRY DllMain( HMODULE hModule,
					   DWORD  ul_reason_for_call,
					   LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		SetD3D8();
		Inject_tests();
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		ClearD3D8();
		break;
	}
	return TRUE;
}
#else
#pragma comment(lib, "dsound.lib")
#define BUFFER_SIZE		32768
#define BUFFER_HALF		(BUFFER_SIZE / 2)
#define BUFFER_QUART	(BUFFER_HALF / 2)

LRESULT WINAPI WndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 1;
	case WM_QUIT:
		DestroyWindow(hWnd);
		break;
	}

	return DefWindowProcW(hWnd, Msg, wParam, lParam);
}

HWND WinInit(HINSTANCE hInst)
{
	WNDCLASSW cl = { 0 };
	
	cl.lpszClassName = L"sh2test";
	cl.lpfnWndProc = WndProc;
	cl.hInstance = hInst;
	cl.hbrBackground = HBRUSH(GetStockObject(COLOR_WINDOW + 1));

	RegisterClassW(&cl);

	return CreateWindowW(L"sh2test", L"ADX TEST", WS_OVERLAPPEDWINDOW, 0, 0, 640, 480, nullptr, nullptr, hInst, nullptr);
}

#define TEST_ADX	0
#define TEST_AIX	1
#define TEST_AFS	0

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	ADXF_LoadPartitionNw(0, "data\\sound\\adx\\voice\\voice.afs", nullptr, nullptr);

	HWND hWnd = WinInit(hInstance);
	ShowWindow(hWnd, SW_SHOW);

	LPDIRECTSOUND8 pDS;
	DirectSoundCreate8(nullptr, &pDS, nullptr);
	if(FAILED(pDS->SetCooperativeLevel(hWnd, DSSCL_EXCLUSIVE)))
		MessageBoxA(hWnd, "failed", "ERRA", MB_OK);

	LPDIRECTSOUNDBUFFER pDB_main;
	DSBUFFERDESC desc = {0};

	// primary buffer
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	desc.dwBufferBytes = 0;
	desc.lpwfxFormat = NULL;
	pDS->CreateSoundBuffer(&desc, &pDB_main, nullptr);

	ADXWIN_SetupSound(pDS);
	ADXM_SetupThrd();

#if TEST_AIX
	DWORD work[2];
	AIXP_Object* aix = AIXP_Create(8, 2, work, sizeof(work));
	AIXP_StartFname(aix, "data\\sound\\adx\\hotel\\bgm_113.aix", nullptr);
#elif TEST_ADX
	ADXT_Object* adx = ADXT_Create(0, nullptr, 0);
	ADXT_StartFname(adx, "data\\sound\\adx\\town\\bgm_005.adx");
#elif TEST_AFS
	ADXF_LoadPartitionNw(0, "data\\sound\\adx\\voice\\voice.afs", nullptr, nullptr);
	ADXT_Object* afs = ADXT_Create(0, nullptr, 0);
	ADXT_StartAfs(afs, 0, 2);
#endif

	MSG msg;
	bool loop = true;
	while (loop)
	{
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				loop = false;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		ADXM_ExecMain();

		//HDC dc = GetDC(hWnd);
		//char mes[32];
		//sprintf_s(mes, sizeof(mes), "Pos %d/%d", ctx.adx->sample_index, in->total_samples);
		//TextOutA(dc, 10, 10, mes, strlen(mes));
		//ReleaseDC(hWnd, dc);
	}

#if TEST_AIX
	AIXP_Destroy(aix);
#elif TEST_ADX
	ADXT_Stop(adx);
#elif TEST_AFS
	ADXT_Stop(afs);
#endif

	ADXM_ShutdownThrd();

	pDB_main->Release();
	pDS->Release();
}
#endif
