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

typedef struct Thread_data
{
	LPDIRECTSOUNDBUFFER pDB;
	CriFileStream* adx;
	u_long nBlockAlign;
	int looping;
} Thread_data;

DWORD WINAPI test_thread(LPVOID data)
{
	Thread_data* ctx = (Thread_data*)data;

	DWORD pos, bytes1, bytes2;
	short *ptr1, *ptr2;

	DWORD snd_dwOffset = 0;
	while (ctx->looping)
	{
		DWORD add = 0;
		const DWORD snd_dwBytes = BUFFER_QUART;

		ctx->pDB->GetCurrentPosition(&pos, nullptr);

		if (pos - snd_dwOffset < 0)
			add = BUFFER_SIZE;
		if (pos + add - snd_dwOffset > 2 * snd_dwBytes + 16)
		{
			ctx->pDB->Lock(snd_dwOffset, snd_dwBytes, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
			decode_adx_standard(ctx->adx, ptr1, bytes1 / ctx->nBlockAlign, 1);
			ctx->pDB->Unlock(ptr1, bytes1, ptr2, bytes2);

			auto total = snd_dwBytes + snd_dwOffset;
			snd_dwOffset = total;
			if (BUFFER_SIZE <= total)
				snd_dwOffset = total - BUFFER_SIZE;
		}
	}

	return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	ADXF_LoadPartitionNw(0, "data\\sound\\adx\\voice\\voice.afs", nullptr, nullptr);

	HWND hWnd = WinInit(hInstance);

	LPDIRECTSOUND8 pDS;
	DirectSoundCreate8(nullptr, &pDS, nullptr);
	if(FAILED(pDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL)))
		MessageBoxA(hWnd, "failed", "ERRA", MB_OK);

	LPDIRECTSOUNDBUFFER pDB_main;
	DSBUFFERDESC desc = {0};

	// primary buffer
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	desc.dwBufferBytes = 0;
	desc.lpwfxFormat = NULL;
	pDS->CreateSoundBuffer(&desc, &pDB_main, nullptr);

	ADXFIC_Create("data\\sound\\adx", 0, nullptr, 0);

	AIX_Handle* aix;
	OpenAIX("data\\sound\\adx\\hotel\\bgm_112.aix", &aix);
	//CloseAIX(aix);

	ADXStream* adx;
	OpenADX("data\\sound\\adx\\apart\\bgm_014.adx", &adx);
	CriFileStream* in0 = (CriFileStream*)adx;
	CriFileStream* in1 = &aix->parent->stream[4];

	ADXWIN_SetupSound(pDS);
	ADXM_SetupThrd();

	auto obj0 = ds_FindObj();
	ds_CreateBuffer(obj0, in1);

	//adxds_SetVolume(obj0, -100);
	//adxds_SetVolume(obj1, -300);

	ds_Play(obj0);
	//adxds_Play(obj1);
	//adxds_Play(obj2);
	//adxds_Play(obj3);
	//adxds_Play(obj4);

	ShowWindow(hWnd, SW_SHOW);

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

		//DWORD pos;
		//pDB->GetCurrentPosition(&pos, nullptr);

		//HDC dc = GetDC(hWnd);
		//char mes[32];
		//sprintf_s(mes, sizeof(mes), "Pos %d/%d", ctx.adx->sample_index, in->total_samples);
		//TextOutA(dc, 10, 10, mes, strlen(mes));
		//ReleaseDC(hWnd, dc);
	}

	//WaitForSingleObject(hThread, INFINITE);
	//pDB->Stop();
	//CloseADX(adx);

	pDB_main->Release();
	pDS->Release();
}
#endif
