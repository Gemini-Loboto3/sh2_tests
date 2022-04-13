#include "dinput8_proxy.h"

/////////////////////////
// Raw Input
HRESULT WINAPI RawInputDevice8::QueryInterface(REFIID riid, LPVOID* ppvObj) { return S_OK; }
ULONG WINAPI RawInputDevice8::AddRef() { return 0; }
ULONG WINAPI RawInputDevice8::Release()
{
	delete this;
	return 0;
}

HRESULT WINAPI RawInputDevice8::GetCapabilities(LPDIDEVCAPS) { return S_OK; }
HRESULT WINAPI RawInputDevice8::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	if (dwFlags & DIDFT_BUTTON)
	{

	}

	DIDEVICEOBJECTINSTANCEA inst;
	inst.dwSize = sizeof(inst);
	inst.dwType = DIDFT_BUTTON | DIDFT_AXIS;
	lpCallback(&inst, pvRef);

	return S_OK;
}
HRESULT WINAPI RawInputDevice8::GetProperty(REFGUID, LPDIPROPHEADER) { return S_OK; }
HRESULT WINAPI RawInputDevice8::SetProperty(REFGUID a, LPCDIPROPHEADER p)
{
	return S_OK;
}

HRESULT WINAPI RawInputDevice8::Acquire()
{
	return S_OK;
}

HRESULT WINAPI RawInputDevice8::Unacquire() { return S_OK; }
HRESULT WINAPI RawInputDevice8::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
	DIJOYSTATE2* st = (DIJOYSTATE2*)lpvData;

	// regular buttons
	st->rgbButtons[0] = p->bButtonStates[0] ? 0x80 : 0;
	st->rgbButtons[1] = p->bButtonStates[1] ? 0x80 : 0;
	st->rgbButtons[2] = p->bButtonStates[2] ? 0x80 : 0;
	st->rgbButtons[3] = p->bButtonStates[3] ? 0x80 : 0;
	st->rgbButtons[4] = p->bButtonStates[4] ? 0x80 : 0;
	st->rgbButtons[5] = p->bButtonStates[5] ? 0x80 : 0;
	st->rgbButtons[6] = p->bButtonStates[6] ? 0x80 : 0;
	st->rgbButtons[7] = p->bButtonStates[7] ? 0x80 : 0;
	st->rgbButtons[8] = p->bButtonStates[8] ? 0x80 : 0;
	st->rgbButtons[9] = p->bButtonStates[9] ? 0x80 : 0;
	st->rgbButtons[10] = p->bButtonStates[10] ? 0x80 : 0;
	st->rgbButtons[11] = p->bButtonStates[11] ? 0x80 : 0;
	// axis
	st->lX = p->lAxisX;
	st->lY = ~p->lAxisY;
	st->lRx = p->lAxisRx;
	st->lRy = ~p->lAxisRy;

	// pov
#define LEFT		0x20
#define RIGHT		0xffe0
#define UP			0x20
#define DOWN		0xffe0
#define NONE		0x0000

	typedef struct tagPAIR
	{
		LONG x, y;
	} PAIR;
	static PAIR t[] =
	{
		NONE, UP,		// 0 up
		RIGHT, UP,		// 1 up+right
		RIGHT, NONE,	// 2 right
		RIGHT, DOWN,	// 3 down+right
		NONE, DOWN,		// 4 down
		LEFT, DOWN,		// 5 down+left
		LEFT, NONE,		// 6 left
		LEFT, UP,		// 7 up+left
		NONE, NONE,		// 8 center
	};
	st->rgdwPOV[0];

	return S_OK;
}

HRESULT WINAPI RawInputDevice8::GetDeviceData(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) { return S_OK; }

HRESULT WINAPI RawInputDevice8::SetDataFormat(LPCDIDATAFORMAT lpdf /*c_dfDIJoystick2*/)
{
	return S_OK;
}
HRESULT WINAPI RawInputDevice8::SetEventNotification(HANDLE) { return S_OK; }
HRESULT WINAPI RawInputDevice8::SetCooperativeLevel(HWND, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::GetDeviceInfo(LPDIDEVICEINSTANCEA) { return S_OK; }
HRESULT WINAPI RawInputDevice8::RunControlPanel(HWND, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::Initialize(HINSTANCE, DWORD, REFGUID) { return S_OK; }
HRESULT WINAPI RawInputDevice8::CreateEffect(REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT* e, LPUNKNOWN)
{
	*e = (LPDIRECTINPUTEFFECT)&effect;
	return S_OK;
}
HRESULT WINAPI RawInputDevice8::EnumEffects(LPDIENUMEFFECTSCALLBACKA cb, LPVOID p, DWORD)
{
	DIEFFECTINFOA info = { 0 };
	cb(&info, p);
	return S_OK;
}
HRESULT WINAPI RawInputDevice8::GetEffectInfo(LPDIEFFECTINFOA, REFGUID) { return S_OK; }
HRESULT WINAPI RawInputDevice8::GetForceFeedbackState(LPDWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::SendForceFeedbackCommand(DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::Escape(LPDIEFFESCAPE) { return S_OK; }
HRESULT WINAPI RawInputDevice8::Poll()
{
	return S_OK;
}

HRESULT WINAPI RawInputDevice8::SendDeviceData(DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::EnumEffectsInFile(LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::WriteEffectToFile(LPCSTR, DWORD, LPDIFILEEFFECT, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::BuildActionMap(LPDIACTIONFORMATA, LPCSTR, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::SetActionMap(LPDIACTIONFORMATA, LPCSTR, DWORD) { return S_OK; }
HRESULT WINAPI RawInputDevice8::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA) { return S_OK; }
