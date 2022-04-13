#include "dinput8_proxy.h"
#pragma comment(lib, "dinput8.lib")

int Device_id = 0;

HRESULT WINAPI DirectInput8CreateProxy(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID* ppvOut, LPUNKNOWN punkOuter)
{
	auto hr = DirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	// create the stuff necessary for proxying
	DInput8Proxy* pDI = new DInput8Proxy((IDirectInput8A*)*ppvOut);
	*ppvOut = (LPVOID)pDI;

	return hr;
}

BOOL PASCAL cbEnumDinput(LPCDIDEVICEINSTANCEA lpddi, LPVOID pvRef)
{
	// ignore sony controllers
	if (_stricmp(lpddi->tszProductName, "Wireless Controller") == 0)
		return 1;
	// ignore xbox 360/one controllers
	if (_stricmp(lpddi->tszProductName, "Controller (XBOX 360 For Windows)") == 0
		|| _stricmp(lpddi->tszProductName, "Controller (XBOX One For Windows)") == 0)
		return 1;

	DInput8Proxy* di = (DInput8Proxy*)pvRef;
	LPDIRECTINPUTDEVICE8A device;
	di->pDi->CreateDevice(lpddi->guidInstance, &device, nullptr);

	if (device)
	{
		DInputDevice8* dev = new DInputDevice8(device);
		di->pad.push_back(dev);
	}
	return 1;
}

static CRawInput* r;
static LRESULT __stdcall RawProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INPUT:
		r->GetInputData(lParam, wParam);
		return 0;
	case WM_INPUT_DEVICE_CHANGE:
		r->DetectRemoval(lParam, wParam);
		return 0;
	}

	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

DInput8Proxy::DInput8Proxy(LPDIRECTINPUT8A dinput)
{
	// enumerate all devices in here
	// enumerate xinput
	for (int i = 0; i < 4; i++)
	{
		Xi[i] = Gamepad(i + 1);
		if (Xi[i].Connected())
		{
			XInputDevice8* x = new XInputDevice8(&Xi[i]);
			pad.push_back(x);
		}
	}
	
	// enumerate raw input
	// == generate fake window to receive the input == //
	WNDCLASSW raw = { 0 };
	raw.lpfnWndProc = RawProc;
	raw.lpszClassName = L"hid";
	if (RegisterClassW(&raw))
		wnd = CreateWindowW(L"hid", L"", 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
	// == window generated == //
	Ri.Create(wnd);
	r = &Ri;
	for (size_t i = 0, si = Ri.device.size(); i < si; i++)
	{
		// enlist only Ds3-4-5
		if (Ri.device[i]->Ds3IsOpen() || Ri.device[i]->Ds4IsOpen())
		{
			RawInputDevice8* r = new RawInputDevice8(Ri.device[i]);
			pad.push_back(r);
		}
	}

	// enumerate direct input
	pDi = dinput;
	pDi->EnumDevices(DI8DEVCLASS_GAMECTRL, cbEnumDinput, this, DIEDFL_ATTACHEDONLY);
}

HRESULT DInput8Proxy::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
	return S_OK;
}
ULONG DInput8Proxy::AddRef()
{
	return pDi->AddRef();
}

ULONG DInput8Proxy::Release()
{
	auto hr = pDi->Release();

	DestroyWindow(wnd);
	if (hr == 0)
		delete this;

	return hr;
}

#define DEF_GUID(x, a, b, c, d, e, f, g, h, i, j, k)	const GUID x = {a,b,c,d,e,f,g,h,i,j,k}

DEF_GUID(GUID_SysMouse, 0x6F1D2B60, 0xD5A0, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);
DEF_GUID(GUID_SysKeyboard, 0x6F1D2B61, 0xD5A0, 0x11CF, 0xBF, 0xC7, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00);

/*** IDirectInput8A methods ***/
HRESULT DInput8Proxy::CreateDevice(REFGUID a, LPDIRECTINPUTDEVICE8A* b, LPUNKNOWN c)
{
	// pure dinput devices that need no wrapping
	if (a == GUID_SysKeyboard || a == GUID_SysMouse)
		return pDi->CreateDevice(a, b, c);

	// anything else (i.e. controllers)
	*b = (LPDIRECTINPUTDEVICE8A)pad[Device_id];
	switch (pad[Device_id]->uType)
	{
	// no special handling
	case DInputDevice8Proxy::TYPE_DINPUT:
	case DInputDevice8Proxy::TYPE_XINPUT:
		break;
	// special handling for selecting a raw input device
	case DInputDevice8Proxy::TYPE_RAWINPUT:
		{
			RawInputDevice8* r = (RawInputDevice8*)pad[Device_id];
			Ri.cur_device = r->p;
		}
		break;
	}
	return S_OK;

	//return pDi->CreateDevice(a, b, pad[Device_id]);
}

HRESULT DInput8Proxy::EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
{
	// do our thing only with game controllers
	if (dwDevType == DI8DEVCLASS_GAMECTRL)
	{
		DIDEVICEINSTANCEA inst = { 0 };
		inst.dwSize = sizeof(inst);
		inst.guidInstance = { 0 };
		inst.guidProduct = { 0 };
		inst.dwDevType = DI8DEVCLASS_GAMECTRL;
		inst.guidFFDriver = { 0 };
		inst.wUsagePage;
		inst.wUsage;

		switch (pad[Device_id]->uType)
		{
		case DInputDevice8Proxy::TYPE_DINPUT:
			break;
		case DInputDevice8Proxy::TYPE_XINPUT:
			strncpy_s(inst.tszInstanceName, "XBox Controller", sizeof(inst.tszInstanceName));
			strncpy_s(inst.tszProductName,  "XBox Controller", sizeof(inst.tszProductName));
			break;
		case DInputDevice8Proxy::TYPE_RAWINPUT:
			strncpy_s(inst.tszInstanceName, ((RawInputDevice8*)pad[Device_id])->p->name.c_str(), sizeof(inst.tszInstanceName));
			strncpy_s(inst.tszProductName,  ((RawInputDevice8*)pad[Device_id])->p->name.c_str(), sizeof(inst.tszProductName));
			break;
		}

		// requires vibration, dinput can't do this
		if (dwFlags & DIEDFL_FORCEFEEDBACK)
		{
			if (pad[Device_id]->uType == DInputDevice8Proxy::TYPE_RAWINPUT ||
				pad[Device_id]->uType == DInputDevice8Proxy::TYPE_XINPUT)
			{
				lpCallback(&inst, pvRef);
				return S_OK;
			}
		}
		else
		{
			if (pad[Device_id]->uType == DInputDevice8Proxy::TYPE_DINPUT)
			{
				lpCallback(&inst, pvRef);
				return S_OK;
			}
		}
		return E_FAIL;
	}
	else return pDi->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

HRESULT DInput8Proxy::GetDeviceStatus(REFGUID rguidInstance) { return pDi->GetDeviceStatus(rguidInstance); }
HRESULT DInput8Proxy::RunControlPanel(HWND a, DWORD b) { return pDi->RunControlPanel(a, b); }
HRESULT DInput8Proxy::Initialize(HINSTANCE a, DWORD b) { return pDi->Initialize(a, b); }
HRESULT DInput8Proxy::FindDevice(REFGUID a, LPCSTR b, LPGUID c) { return pDi->FindDevice(a, b, c); }
HRESULT DInput8Proxy::EnumDevicesBySemantics(LPCSTR a, LPDIACTIONFORMATA b, LPDIENUMDEVICESBYSEMANTICSCBA c, LPVOID d, DWORD e) { return pDi->EnumDevicesBySemantics(a, b, c, d, e); }
HRESULT DInput8Proxy::ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK a, LPDICONFIGUREDEVICESPARAMSA b, DWORD c, LPVOID d) { return pDi->ConfigureDevices(a, b, c, d); }

/*** extended methods for processing raw input ***/
void DInput8Proxy::ProcessRaw(LPARAM lParam, WPARAM wParam)
{
	Ri.GetInputData(lParam, wParam);
}
void DInput8Proxy::UpdateRaw(LPARAM lParam, WPARAM wParam){}

///////////////////////////////////////////////
/*** IUnknown methods ***/
HRESULT WINAPI DInputDevice8Proxy::QueryInterface(REFIID riid, LPVOID* ppvObj) { return S_OK; }
ULONG WINAPI DInputDevice8Proxy::AddRef() { return 0; }
ULONG WINAPI DInputDevice8Proxy::Release() { return 0; }

/*** IDirectInputDevice8A methods ***/
HRESULT WINAPI DInputDevice8Proxy::GetCapabilities(LPDIDEVCAPS) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::GetProperty(REFGUID, LPDIPROPHEADER) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::SetProperty(REFGUID, LPCDIPROPHEADER) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::Acquire() { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::Unacquire() { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::GetDeviceState(DWORD, LPVOID) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::GetDeviceData(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::SetDataFormat(LPCDIDATAFORMAT) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::SetEventNotification(HANDLE) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::SetCooperativeLevel(HWND, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::GetDeviceInfo(LPDIDEVICEINSTANCEA) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::RunControlPanel(HWND, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::Initialize(HINSTANCE, DWORD, REFGUID) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::CreateEffect(REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT*, LPUNKNOWN) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::EnumEffects(LPDIENUMEFFECTSCALLBACKA, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::GetEffectInfo(LPDIEFFECTINFOA, REFGUID) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::GetForceFeedbackState(LPDWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::SendForceFeedbackCommand(DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::Escape(LPDIEFFESCAPE) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::Poll() { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::SendDeviceData(DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::EnumEffectsInFile(LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::WriteEffectToFile(LPCSTR, DWORD, LPDIFILEEFFECT, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::BuildActionMap(LPDIACTIONFORMATA, LPCSTR, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::SetActionMap(LPDIACTIONFORMATA, LPCSTR, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8Proxy::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA) { return S_OK; }

///////////////////////////////
// XInput
HRESULT XInputEffect::SetParameters(LPCDIEFFECT peff, DWORD dwFlags)
{
	if (peff)
		eff = *peff;
	
	DIEP_DIRECTION;

	return S_OK;
}

HRESULT XInputEffect::Start(DWORD dwIterations, DWORD dwFlags)
{
	dev->vibrate(eff.dwGain / 10000.f, eff.dwGain / 10000.f);
	return S_OK;
}

HRESULT XInputEffect::Stop()
{
	dev->vibrate(0.f, 0.f);
	return S_OK;
}

HRESULT WINAPI XInputDevice8::QueryInterface(REFIID riid, LPVOID* ppvObj) { return S_OK; }
ULONG WINAPI XInputDevice8::AddRef() { return 0; }
ULONG WINAPI XInputDevice8::Release()
{
	delete this;
	return 0;
}

HRESULT WINAPI XInputDevice8::GetCapabilities(LPDIDEVCAPS) { return S_OK; }
HRESULT WINAPI XInputDevice8::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags)
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
HRESULT WINAPI XInputDevice8::GetProperty(REFGUID, LPDIPROPHEADER) { return S_OK; }
HRESULT WINAPI XInputDevice8::SetProperty(REFGUID a, LPCDIPROPHEADER p)
{
	return S_OK;
}

HRESULT WINAPI XInputDevice8::Acquire()
{
	return S_OK;
}

HRESULT WINAPI XInputDevice8::Unacquire() { return S_OK; }
HRESULT WINAPI XInputDevice8::GetDeviceState(DWORD cbData, LPVOID lpvData)
{
	DIJOYSTATE2 *st = (DIJOYSTATE2*)lpvData;

	p->Update();

	// regular buttons
	st->rgbButtons[0] = p->GetButtonPressed(xButtons.A)            ? 0x80 : 0;
	st->rgbButtons[1] = p->GetButtonPressed(xButtons.B)            ? 0x80 : 0;
	st->rgbButtons[2] = p->GetButtonPressed(xButtons.X)            ? 0x80 : 0;
	st->rgbButtons[3] = p->GetButtonPressed(xButtons.Y)            ? 0x80 : 0;
	st->rgbButtons[4] = p->GetButtonPressed(xButtons.Start)        ? 0x80 : 0;
	st->rgbButtons[5] = p->GetButtonPressed(xButtons.Back)         ? 0x80 : 0;
	st->rgbButtons[6] = p->GetButtonPressed(xButtons.L_Shoulder)   ? 0x80 : 0;
	st->rgbButtons[7] = p->GetButtonPressed(xButtons.R_Shoulder)   ? 0x80 : 0;
	st->rgbButtons[8] = p->GetButtonPressed(xButtons.L_Thumbstick) ? 0x80 : 0;
	st->rgbButtons[9] = p->GetButtonPressed(xButtons.R_Thumbstick) ? 0x80 : 0;
	st->rgbButtons[10] = p->LeftTrigger() ? 0x80 : 0;
	st->rgbButtons[11] = p->RightTrigger() ? 0x80 : 0;
	// axis
	st->lX = p->LeftStick_Xs();
	st->lY = ~p->LeftStick_Ys();
	st->lRx = p->RightStick_Xs();
	st->lRy = ~p->RightStick_Ys();

	return S_OK;
}

HRESULT WINAPI XInputDevice8::GetDeviceData(DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD) { return S_OK; }

HRESULT WINAPI XInputDevice8::SetDataFormat(LPCDIDATAFORMAT lpdf /*c_dfDIJoystick2*/)
{
	return S_OK;
}
HRESULT WINAPI XInputDevice8::SetEventNotification(HANDLE) { return S_OK; }
HRESULT WINAPI XInputDevice8::SetCooperativeLevel(HWND, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA, DWORD, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::GetDeviceInfo(LPDIDEVICEINSTANCEA) { return S_OK; }
HRESULT WINAPI XInputDevice8::RunControlPanel(HWND, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::Initialize(HINSTANCE, DWORD, REFGUID) { return S_OK; }
HRESULT WINAPI XInputDevice8::CreateEffect(REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
{
	effect.dev = this;
	*ppdeff = (LPDIRECTINPUTEFFECT)&effect;
	return S_OK;
}
HRESULT WINAPI XInputDevice8::EnumEffects(LPDIENUMEFFECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwEffType)
{
	DIEFFECTINFOA info = { 0 };
	lpCallback(&info, pvRef);
	return S_OK;
}
HRESULT WINAPI XInputDevice8::GetEffectInfo(LPDIEFFECTINFOA, REFGUID) { return S_OK; }
HRESULT WINAPI XInputDevice8::GetForceFeedbackState(LPDWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::SendForceFeedbackCommand(DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::Escape(LPDIEFFESCAPE) { return S_OK; }
HRESULT WINAPI XInputDevice8::Poll()
{
	return S_OK;
}

HRESULT WINAPI XInputDevice8::SendDeviceData(DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::EnumEffectsInFile(LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::WriteEffectToFile(LPCSTR, DWORD, LPDIFILEEFFECT, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::BuildActionMap(LPDIACTIONFORMATA, LPCSTR, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::SetActionMap(LPDIACTIONFORMATA, LPCSTR, DWORD) { return S_OK; }
HRESULT WINAPI XInputDevice8::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA) { return S_OK; }

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

////////////////////////////////
// Direct Input (complete proxy)
HRESULT WINAPI DInputDevice8::QueryInterface(REFIID riid, LPVOID* ppvObj) { return di->QueryInterface(riid, ppvObj); }
ULONG   WINAPI DInputDevice8::AddRef() { return di->AddRef(); }
ULONG   WINAPI DInputDevice8::Release()
{
	auto hr = di->Release();
	delete this;
	return hr;
}
HRESULT WINAPI DInputDevice8::GetCapabilities(LPDIDEVCAPS a) { return di->GetCapabilities(a); }
HRESULT WINAPI DInputDevice8::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) { return di->EnumObjects(lpCallback, pvRef, dwFlags); }
HRESULT WINAPI DInputDevice8::GetProperty(REFGUID a, LPDIPROPHEADER b) { return di->GetProperty(a, b); }
HRESULT WINAPI DInputDevice8::SetProperty(REFGUID a, LPCDIPROPHEADER p) { return di->SetProperty(a, p); }
HRESULT WINAPI DInputDevice8::Acquire() { return di->Acquire(); }
HRESULT WINAPI DInputDevice8::Unacquire() { return di->Unacquire(); }
HRESULT WINAPI DInputDevice8::GetDeviceState(DWORD cbData, LPVOID lpvData) { return di->GetDeviceState(cbData, lpvData); }
HRESULT WINAPI DInputDevice8::GetDeviceData(DWORD a, LPDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d) { return di->GetDeviceData(a, b, c, d); }
HRESULT WINAPI DInputDevice8::SetDataFormat(LPCDIDATAFORMAT lpdf) { return di->SetDataFormat(lpdf); }
HRESULT WINAPI DInputDevice8::SetEventNotification(HANDLE a) { return di->SetEventNotification(a); }
HRESULT WINAPI DInputDevice8::SetCooperativeLevel(HWND a, DWORD b) { return di->SetCooperativeLevel(a,b); }
HRESULT WINAPI DInputDevice8::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA a, DWORD b, DWORD c) { return di->GetObjectInfo(a, b, c); }
HRESULT WINAPI DInputDevice8::GetDeviceInfo(LPDIDEVICEINSTANCEA a) { return di->GetDeviceInfo(a); }
HRESULT WINAPI DInputDevice8::RunControlPanel(HWND a, DWORD b) { return di->RunControlPanel(a, b); }
HRESULT WINAPI DInputDevice8::Initialize(HINSTANCE a, DWORD b, REFGUID c) { return di->Initialize(a, b, c); }
HRESULT WINAPI DInputDevice8::CreateEffect(REFGUID a, LPCDIEFFECT b, LPDIRECTINPUTEFFECT* c, LPUNKNOWN d) { return di->CreateEffect(a, b, c, d); }
HRESULT WINAPI DInputDevice8::EnumEffects(LPDIENUMEFFECTSCALLBACKA cb, LPVOID p, DWORD c) { return di->EnumEffects(cb, p, c); }
HRESULT WINAPI DInputDevice8::GetEffectInfo(LPDIEFFECTINFOA a, REFGUID b) { return di->GetEffectInfo(a, b); }
HRESULT WINAPI DInputDevice8::GetForceFeedbackState(LPDWORD a) { return di->GetForceFeedbackState(a); }
HRESULT WINAPI DInputDevice8::SendForceFeedbackCommand(DWORD a) { return di->SendForceFeedbackCommand(a); }
HRESULT WINAPI DInputDevice8::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD) { return S_OK; }
HRESULT WINAPI DInputDevice8::Escape(LPDIEFFESCAPE a) { return di->Escape(a); }
HRESULT WINAPI DInputDevice8::Poll() { return di->Poll(); }
HRESULT WINAPI DInputDevice8::SendDeviceData(DWORD a, LPCDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d) { return di->SendDeviceData(a, b, c, d); }
HRESULT WINAPI DInputDevice8::EnumEffectsInFile(LPCSTR a, LPDIENUMEFFECTSINFILECALLBACK b, LPVOID c, DWORD d) { return di->EnumEffectsInFile(a, b, c, d); }
HRESULT WINAPI DInputDevice8::WriteEffectToFile(LPCSTR a, DWORD b, LPDIFILEEFFECT c, DWORD d) { return di->WriteEffectToFile(a, b, c, d); }
HRESULT WINAPI DInputDevice8::BuildActionMap(LPDIACTIONFORMATA a, LPCSTR b, DWORD c) { return di->BuildActionMap(a, b, c); }
HRESULT WINAPI DInputDevice8::SetActionMap(LPDIACTIONFORMATA a, LPCSTR b, DWORD c) { return di->SetActionMap(a, b, c); }
HRESULT WINAPI DInputDevice8::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA a) { return di->GetImageInfo(a); }
