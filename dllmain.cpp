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

	int flip = 1;
	DWORD pos, bytes1, bytes2;
	short *ptr1, *ptr2;

	while (ctx->looping)
	{
		ctx->pDB->GetCurrentPosition(&pos, nullptr);

		if (pos < BUFFER_HALF && flip == 0)
		{
			flip = 1;
			ctx->pDB->Lock(BUFFER_HALF, BUFFER_HALF, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
			decode_adx_standard(ctx->adx, ptr1, bytes1 / ctx->nBlockAlign, 1);
			ctx->pDB->Unlock(ptr1, bytes1, ptr2, bytes2);
		}
		else if (pos >= BUFFER_HALF && flip == 1)
		{
			flip = 0;
			ctx->pDB->Lock(0, BUFFER_HALF, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
			decode_adx_standard(ctx->adx, ptr1, bytes1 / ctx->nBlockAlign, 1);
			ctx->pDB->Unlock(ptr1, bytes1, ptr2, bytes2);
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

	LPDIRECTSOUNDBUFFER pDB, pDB_main;
	DSBUFFERDESC desc = {0};
	WAVEFORMATEX fmt;
	HRESULT hr;

	fmt.cbSize = sizeof(fmt);
	fmt.nSamplesPerSec = 44100;
	fmt.nBlockAlign = 4;
	fmt.nChannels = 2;
	fmt.wBitsPerSample = 16;
	fmt.wFormatTag = WAVE_FORMAT_PCM;
	fmt.nAvgBytesPerSec = 44100 * 4;

	// primary buffer
	desc.dwSize = sizeof(desc);
	desc.dwFlags = DSBCAPS_PRIMARYBUFFER;
	desc.dwBufferBytes = 0;
	desc.lpwfxFormat = NULL;
	pDS->CreateSoundBuffer(&desc, &pDB_main, nullptr);
	//if (FAILED(pDB_main->SetFormat(&fmt)))
		/*MessageBoxA(hWnd, "failed\n", "ERRA", MB_OK)*/;

	ADXFIC_Create("data\\sound\\adx", 0, nullptr, 0);

	short *ptr1, *ptr2;
	DWORD bytes1, bytes2;

	AIX* aix;
	OpenAIX("data\\sound\\adx\\hotel\\bgm_113.aix", &aix);
	CloseAIX(aix);

	ADX* adx;
	OpenADX("data\\sound\\adx\\apart\\bgm_014.adx", &adx);
	CriFileStream* in = (CriFileStream*)adx;

	// adx playback buffer
	desc.lpwfxFormat = &fmt;
	desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN | DSBCAPS_CTRLFREQUENCY;
	desc.dwBufferBytes = BUFFER_SIZE;
	fmt.nSamplesPerSec = in->sample_rate;
	fmt.nAvgBytesPerSec = in->sample_rate * 4;
	hr = pDS->CreateSoundBuffer(&desc, &pDB, nullptr);

	// perform first filling
	pDB->Lock(0, BUFFER_SIZE, (LPVOID*)&ptr1, &bytes1, (LPVOID*)&ptr2, &bytes2, 0);
	decode_adx_standard((CriFileStream*)adx, ptr1, bytes1 / fmt.nBlockAlign, 1);
	pDB->Unlock(ptr1, bytes1, ptr2, bytes2);
	
	Thread_data ctx;
	ctx.pDB = pDB;
	ctx.adx = (CriFileStream*)adx;
	ctx.nBlockAlign = fmt.nBlockAlign;
	HANDLE hThread = CreateThread(nullptr, 0, test_thread, (LPVOID)&ctx, CREATE_SUSPENDED, nullptr);
	SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);

	int flip = 1;
	// kick playback
	pDB->SetVolume(-1000);
	pDB->Play(0, 0, DSBPLAY_LOOPING);
	ResumeThread(hThread);

	ShowWindow(hWnd, SW_SHOW);

	MSG msg;
	bool loop = true;
	while (loop)
	{
		while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				ctx.looping = false;
				loop = false;
			}
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		DWORD pos;
		pDB->GetCurrentPosition(&pos, nullptr);

		HDC dc = GetDC(hWnd);
		char mes[32];
		sprintf_s(mes, sizeof(mes), "Pos %d/%d", ctx.adx->sample_index, in->total_samples);
		TextOutA(dc, 10, 10, mes, strlen(mes));
		ReleaseDC(hWnd, dc);
	}

	WaitForSingleObject(hThread, INFINITE);
	pDB->Stop();
	CloseADX(adx);

	pDB->Release();
	pDB_main->Release();
	pDS->Release();
}
#endif
