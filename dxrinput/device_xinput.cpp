#include "dinput8_proxy.h"

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
	DIJOYSTATE2* st = (DIJOYSTATE2*)lpvData;

	p->Update();

	// regular buttons
	st->rgbButtons[0] = p->GetButtonPressed(xButtons.A) ? 0x80 : 0;
	st->rgbButtons[1] = p->GetButtonPressed(xButtons.B) ? 0x80 : 0;
	st->rgbButtons[2] = p->GetButtonPressed(xButtons.X) ? 0x80 : 0;
	st->rgbButtons[3] = p->GetButtonPressed(xButtons.Y) ? 0x80 : 0;
	st->rgbButtons[4] = p->GetButtonPressed(xButtons.Start) ? 0x80 : 0;
	st->rgbButtons[5] = p->GetButtonPressed(xButtons.Back) ? 0x80 : 0;
	st->rgbButtons[6] = p->GetButtonPressed(xButtons.L_Shoulder) ? 0x80 : 0;
	st->rgbButtons[7] = p->GetButtonPressed(xButtons.R_Shoulder) ? 0x80 : 0;
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