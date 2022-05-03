#include <windows.h>
#include "daudio.h"

static HMODULE xdll = nullptr;
static HRESULT(__stdcall* XAudio2CreateEx)(_Outptr_ IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);

static IXAudio2* pXAudio2;
static IXAudio2MasteringVoice* pMasterVoice;

// XAudio 2.9 dll loader
static int Xaudio2_loader()
{
	if (xdll)
		return S_OK;

#if _DEBUG
	xdll = LoadLibraryW(L"xaudio2_9d.dll");
#else
	xdll = LoadLibraryW(L"xaudio2_9.dll");
#endif
	// if it's not in the game folder, go for system32
	if (!xdll)
	{
		wchar_t path[MAX_PATH], path_w32[MAX_PATH];
		GetSystemDirectoryW(path_w32, MAX_PATH);
		wsprintf(path, L"%s\\xaudio2_9.dll", path_w32);
		xdll = LoadLibraryW(path);
		if (!xdll)
			return S_FALSE;
	}

	// grab XAudio2 main procedure and we're good
	XAudio2CreateEx = (HRESULT(__stdcall*)(IXAudio2**, UINT32, XAUDIO2_PROCESSOR))GetProcAddress(xdll, "XAudio2Create");
	return S_OK;
}

HRESULT WINAPI DirectAudioCreate(LPCGUID pcGuidDevice, IDirectAudio** ppDA, LPUNKNOWN pUnkOuter)
{
	Xaudio2_loader();

	IXAudio2* XA;
	XAudio2CreateEx(&XA, 0, 0);

	auto pDA = new IDirectAudio(XA);

	*ppDA = pDA;

	return S_OK;
}

//----------------------------------------------
HRESULT IDirectAudio::QueryInterface(REFIID refid, LPVOID* pp)
{
	if (refid == IID_IXAudio2)
		*pp = pXA;
	else if (refid == IID_IDirectSound || refid == IID_IDirectSound8)
		*pp = this;

	dwSpeakerConf = DASPEAKER_STEREO;

	return S_OK;
}

ULONG IDirectAudio::AddRef()
{
	return ++RefVal;
}

ULONG IDirectAudio::Release()
{
	u_long ret = --RefVal;

	delete this;

	return --ret;
}

// The CreateSoundBuffer method creates a sound buffer object to manage audio samples.
HRESULT IDirectAudio::CreateSoundBuffer(const LPDABUFFERDESC pcDSBufferDesc, IDirectAudioBuffer** ppDSBuffer, LPUNKNOWN pUnkOuter)
{
	IDirectAudioBuffer* buf = new IDirectAudioBuffer(this, pcDSBufferDesc);

	*ppDSBuffer = buf;
	AddRef();

	return S_OK;
}

// The GetCaps method retrieves the capabilities of the hardware device that is represented by the device object.
HRESULT IDirectAudio::GetCaps(LPDACAPS pDSCaps)
{
	pDSCaps->dwFlags = DACAPS_PRIMARYMONO | DACAPS_PRIMARYSTEREO | DACAPS_PRIMARY8BIT | DACAPS_PRIMARY16BIT | DACAPS_CONTINUOUSRATE |
		DACAPS_EMULDRIVER | DACAPS_CERTIFIED | DACAPS_SECONDARYMONO | DACAPS_SECONDARYSTEREO | DACAPS_SECONDARY8BIT | DACAPS_SECONDARY16BIT;
	pDSCaps->dwMinSecondarySampleRate = 4000;
	pDSCaps->dwMaxSecondarySampleRate = 44100;
	pDSCaps->dwPrimaryBuffers = 1;
	pDSCaps->dwTotalHwMemBytes = 0x7fffffff;
	pDSCaps->dwFreeHwMemBytes = 0x7fffffff;
	pDSCaps->dwMaxHw3DAllBuffers = 32;

	memset(pDSCaps, 0, pDSCaps->dwSize);

	return S_OK;
}

// The DuplicateSoundBuffer method creates a new secondary buffer that shares the original buffer's memory.
HRESULT IDirectAudio::DuplicateSoundBuffer(IDirectAudioBuffer* pDSBufferOriginal, IDirectAudioBuffer** ppDSBufferDuplicate)
{
	return S_OK;
}

// The SetCooperativeLevel method sets the cooperative level of the application for this sound device.
HRESULT IDirectAudio::SetCooperativeLevel(HWND hwnd, DWORD dwLevel)
{
	return S_OK;
}

// The Compact method has no effect.
HRESULT IDirectAudio::Compact()
{
	return S_OK;
}

// The GetSpeakerConfig method retrieves the speaker configuration.
HRESULT IDirectAudio::GetSpeakerConfig(LPDWORD pdwSpeakerConfig)
{
	if (pdwSpeakerConfig) *pdwSpeakerConfig = dwSpeakerConf;

	return S_OK;
}

// The SetSpeakerConfig method specifies the speaker configuration of the device.
HRESULT IDirectAudio::SetSpeakerConfig(DWORD dwSpeakerConfig)
{
	dwSpeakerConf = dwSpeakerConfig;

	switch (dwSpeakerConfig & 0xffff)
	{
	case DASPEAKER_DIRECTOUT:
	case DASPEAKER_HEADPHONE:
		break;
	case DASPEAKER_MONO:
	case DASPEAKER_QUAD:
	case DASPEAKER_STEREO:
		switch (dwSpeakerConf >> 16)
		{
		case DASPEAKER_GEOMETRY_MIN:	//   5 degrees
		case DASPEAKER_GEOMETRY_NARROW:	//  10 degrees
		case DASPEAKER_GEOMETRY_WIDE:	//  20 degrees
		case DASPEAKER_GEOMETRY_MAX:	// 180 degrees
			break;
		}
		break;
	case DASPEAKER_SURROUND:
	case DASPEAKER_7POINT1_SURROUND:	// correct 7.1 Home Theater setting
	case DASPEAKER_5POINT1_SURROUND:	// correct 5.1 setting

	case DASPEAKER_5POINT1:	// obsolete 5.1 setting
	case DASPEAKER_7POINT1:	// obsolete 7.1 setting
		break;
	}

	return S_OK;
}

// The Initialize method initializes a device object that was created by using the CoCreateInstance function.
HRESULT IDirectAudio::Initialize(LPCGUID pcGuidDevice)
{
	return S_OK;
}

// The VerifyCertification method ascertains whether the device driver is certified for DirectX.
HRESULT IDirectAudio::VerifyCertification(LPDWORD pdwCertified)
{
	return S_OK;
}
