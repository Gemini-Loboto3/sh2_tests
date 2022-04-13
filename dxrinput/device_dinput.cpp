#include "dinput8_proxy.h"

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
HRESULT WINAPI DInputDevice8::SetCooperativeLevel(HWND a, DWORD b) { return di->SetCooperativeLevel(a, b); }
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
