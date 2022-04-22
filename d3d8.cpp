#include "framework.h"
#include "d3d8_proxy.h"
#include <d3d9.h>

HMODULE dll = nullptr, proxy = nullptr;
FARPROC fp[4];

//void _DebugSetMute()
//{
//}

void SetD3D8()
{
	char path[MAX_PATH];
	CopyMemory(path + GetSystemDirectoryA(path, MAX_PATH - 9), "\\d3d8.dll", 10);
	dll = LoadLibraryA(path);
	if (dll)
	{
		fp[0] = GetProcAddress(dll, "ValidatePixelShader");
		fp[1] = GetProcAddress(dll, "ValidateVertexShader");
		fp[2] = GetProcAddress(dll, "DebugSetMute");
		fp[3] = GetProcAddress(dll, "Direct3DCreate8");
	}
}

void ClearD3D8()
{
	if(dll) FreeLibrary(dll);
	if (proxy) FreeLibrary(proxy);
}

void __declspec(naked) ValidatePixelShader()  { __asm {jmp fp[0 * 4]}; }
void __declspec(naked) ValidateVertexShader() { __asm {jmp fp[1 * 4]}; }
void __declspec(naked) _DebugSetMute()        { __asm {jmp fp[2 * 4]}; }
#if 1
void __declspec(naked) _Direct3DCreate8()     { __asm {jmp fp[3 * 4]}; }
#else
typedef IDirect3D8* (WINAPI *create)(UINT SDKVersion);

IDirect3D8* WINAPI _Direct3DCreate8(UINT SDKVersion)
{
	IDirect3D8 *d3d8 = ((create)fp[3])(SDKVersion);		// call the true Direct3DCreate8
	IDirect3D8Proxy* proxy = new IDirect3D8Proxy();
	proxy->d3d8 = d3d8;

	return (IDirect3D8*)proxy;
}
#endif