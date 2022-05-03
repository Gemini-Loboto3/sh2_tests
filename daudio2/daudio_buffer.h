#pragma once
#include "..\xaudio\include\x3daudioredist.h"

typedef float D3DVALUE, * LPD3DVALUE;
#ifndef D3DVECTOR_DEFINED
typedef struct _D3DVECTOR {
	float x;
	float y;
	float z;
} D3DVECTOR;
#endif

typedef struct _DA3DLISTENER
{
	DWORD           dwSize;
	D3DVECTOR       vPosition;
	D3DVECTOR       vVelocity;
	D3DVECTOR       vOrientFront;
	D3DVECTOR       vOrientTop;
	D3DVALUE        flDistanceFactor;
	D3DVALUE        flRolloffFactor;
	D3DVALUE        flDopplerFactor;
} DA3DLISTENER, * LPDA3DLISTENER;

class IDirectAudio3DListener
{
public:
	IDirectAudio3DListener()
	{
		list.OrientFront.x = 0.f;
		list.OrientFront.y = 0.f;
		list.OrientFront.z = 0.f;
		list.OrientTop.x = 0.f;
		list.OrientTop.y = 0.f;
		list.OrientTop.z = 0.f;
		list.Position.x = 0.f;
		list.Position.y = 0.f;
		list.Position.z = 0.f;
		list.Velocity.x = 0.f;
		list.Velocity.y = 0.f;
		list.Velocity.z = 0.f;
		list.pCone = nullptr;
	}

	// IUnknown methods
	STDMETHOD(QueryInterface)           (THIS_ _In_ REFIID, _Outptr_ LPVOID*) { return S_OK; }
	STDMETHOD_(ULONG, AddRef)            (THIS) { return 0; }
	STDMETHOD_(ULONG, Release)           (THIS) { delete this; return 0; }

	// IDirectSound3DListener methods
	STDMETHOD(GetAllParameters)         (THIS_ _Out_ LPDA3DLISTENER pListener) { return S_OK; }
	STDMETHOD(GetDistanceFactor)        (THIS_ _Out_ D3DVALUE* pflDistanceFactor) { return S_OK; }
	STDMETHOD(GetDopplerFactor)         (THIS_ _Out_ D3DVALUE* pflDopplerFactor) { return S_OK; }
	STDMETHOD(GetOrientation)           (THIS_ _Out_ D3DVECTOR* pvOrientFront, _Out_ D3DVECTOR* pvOrientTop) { return S_OK; }
	STDMETHOD(GetPosition)              (THIS_ _Out_ D3DVECTOR* pvPosition) { return S_OK; }
	STDMETHOD(GetRolloffFactor)         (THIS_ _Out_ D3DVALUE* pflRolloffFactor) { return S_OK; }
	STDMETHOD(GetVelocity)              (THIS_ _Out_ D3DVECTOR* pvVelocity) { return S_OK; }
	STDMETHOD(SetAllParameters)         (THIS_ _In_ const LPDA3DLISTENER pcListener, DWORD dwApply) { return S_OK; }
	STDMETHOD(SetDistanceFactor)        (THIS_ D3DVALUE flDistanceFactor, DWORD dwApply) { return S_OK; }
	STDMETHOD(SetDopplerFactor)         (THIS_ D3DVALUE flDopplerFactor, DWORD dwApply) { return S_OK; }
	STDMETHOD(SetOrientation)           (THIS_ D3DVALUE xFront, D3DVALUE yFront, D3DVALUE zFront, D3DVALUE xTop, D3DVALUE yTop, D3DVALUE zTop, DWORD dwApply) { return S_OK; }
	STDMETHOD(SetPosition)              (THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
	{
		list.Position.x = x;
		list.Position.y = y;
		list.Position.z = z;

		return S_OK;
	}
	STDMETHOD(SetRolloffFactor)         (THIS_ D3DVALUE flRolloffFactor, DWORD dwApply)
	{
		return S_OK;
	}
	STDMETHOD(SetVelocity)              (THIS_ D3DVALUE x, D3DVALUE y, D3DVALUE z, DWORD dwApply)
	{
		list.Velocity.x = x;
		list.Velocity.y = y;
		list.Velocity.z = z;

		return S_OK;
	}
	STDMETHOD(CommitDeferredSettings)   (THIS) { return S_OK; }

	X3DAUDIO_LISTENER list;
};

class IDirectAudioBuffer
{
public:
	IDirectAudioBuffer(IDirectAudio* DA, LPDABUFFERDESC desc);

	// IUnknown methods
	STDMETHOD(QueryInterface)        (REFIID, LPVOID*);
	STDMETHOD_(ULONG, AddRef)        ();
	STDMETHOD_(ULONG, Release)       ();

	// IDirectSoundBuffer methods
	STDMETHOD(GetCaps)              (LPDABCAPS pDSBufferCaps);
	STDMETHOD(GetCurrentPosition)   (LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor);
	STDMETHOD(GetFormat)            (LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten);
	STDMETHOD(GetVolume)            (LPLONG plVolume);
	STDMETHOD(GetPan)               (LPLONG plPan);
	STDMETHOD(GetFrequency)         (LPDWORD pdwFrequency);
	STDMETHOD(GetStatus)            (LPDWORD pdwStatus);
	STDMETHOD(Initialize)           (IDirectAudio* pDirectSound, const LPDABUFFERDESC pcDSBufferDesc);
	STDMETHOD(Lock)                 (DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
	STDMETHOD(Play)                 (DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags);
	STDMETHOD(SetCurrentPosition)   (DWORD dwNewPosition);
	STDMETHOD(SetFormat)            (LPCWAVEFORMATEX pcfxFormat);
	STDMETHOD(SetVolume)            (LONG lVolume);
	STDMETHOD(SetPan)               (LONG lPan);
	STDMETHOD(SetFrequency)         (DWORD dwFrequency);
	STDMETHOD(Stop)                 ();
	STDMETHOD(Unlock)               (LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2);
	STDMETHOD(Restore)              ();

private:
	WAVEFORMATEX fmt;
	IDirectAudio* pDA;

	union voice
	{
		IXAudio2Voice *v;
		IXAudio2SourceVoice* sv;
		IXAudio2MasteringVoice* mv;
	};

	voice Voice;
	u_long is_primary : 1;
	BYTE* buffer;
	DWORD dwBytes;
	LONG volume;

	DABUFFERDESC desc;
	IDirectAudio3DListener* pListener;

	friend class IDirectAudio;
	friend class IDirectAudio3DListener;
};
