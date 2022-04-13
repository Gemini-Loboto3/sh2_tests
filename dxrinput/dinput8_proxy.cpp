#include "dinput8_proxy.h"
#pragma comment(lib, "dinput8.lib")

EXTERN_C HWND hWnd;
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
	// checks to disable xinput and sony controllers
	//if (_stricmp(lpddi->tszProductName, "Wireless Controller") == 0)
	//	return 1;
	//if (_stricmp(lpddi->tszProductName, "Controller (XBOX 360 For Windows)") == 0
	//	|| _stricmp(lpddi->tszProductName, "Controller (XBOX One For Windows)") == 0)
	//	return 1;

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
	//case WM_INPUT_DEVICE_CHANGE:
	//	r->Detection(lParam, wParam);
	//	return 0;
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

	WNDCLASSW raw = { 0 };
	raw.lpfnWndProc = RawProc;
	raw.lpszClassName = L"hid";
	if(RegisterClassW(&raw))
		wnd = CreateWindowW(L"hid", L"", 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);
	
	// enumerate raw input
	Ri.Create(wnd/*hWnd*/);
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
