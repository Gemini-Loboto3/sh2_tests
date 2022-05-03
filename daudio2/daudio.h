#pragma once

#ifndef XAUDIO2_HELPER_FUNCTIONS
#define XAUDIO2_HELPER_FUNCTIONS	1
#endif

#include "..\xaudio\include\xaudio2.h"

DEFINE_GUID(IID_IXAudio2,            0xDEADBEEF, 0xDEAD, 0xBEEF, 0xDE, 0xAD, 0xBE, 0xEF, 0xF0, 0x0D, 0, 0);
DEFINE_GUID(IID_IDirectSound,        0x279AFA83, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IDirectSound8,       0xC50A7E93, 0xF395, 0x4834, 0x9E, 0xF6, 0x7F, 0xA9, 0x9D, 0xE5, 0x09, 0x66);
DEFINE_GUID(IID_IDirectSoundBuffer,  0x279AFA85, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);
DEFINE_GUID(IID_IDirectSoundBuffer8, 0x6825a449, 0x7524, 0x4d82, 0x92, 0x0f, 0x50, 0xe3, 0x6a, 0xb3, 0xab, 0x1e);
DEFINE_GUID(IID_IDirectSound3DListener, 0x279AFA84, 0x4981, 0x11CE, 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60);

// forward declarations
class IDirectAudio;
class IDirectAudioBuffer;

#define DACAPS_PRIMARYMONO          0x00000001
#define DACAPS_PRIMARYSTEREO        0x00000002
#define DACAPS_PRIMARY8BIT          0x00000004
#define DACAPS_PRIMARY16BIT         0x00000008
#define DACAPS_CONTINUOUSRATE       0x00000010
#define DACAPS_EMULDRIVER           0x00000020
#define DACAPS_CERTIFIED            0x00000040
#define DACAPS_SECONDARYMONO        0x00000100
#define DACAPS_SECONDARYSTEREO      0x00000200
#define DACAPS_SECONDARY8BIT        0x00000400
#define DACAPS_SECONDARY16BIT       0x00000800

#define DABCAPS_PRIMARYBUFFER       0x00000001
#define DABCAPS_STATIC              0x00000002
#define DABCAPS_LOCHARDWARE         0x00000004
#define DABCAPS_LOCSOFTWARE         0x00000008
#define DABCAPS_CTRL3D              0x00000010
#define DABCAPS_CTRLFREQUENCY       0x00000020
#define DABCAPS_CTRLPAN             0x00000040
#define DABCAPS_CTRLVOLUME          0x00000080
#define DABCAPS_CTRLPOSITIONNOTIFY  0x00000100
#define DABCAPS_CTRLFX              0x00000200
#define DABCAPS_STICKYFOCUS         0x00004000
#define DABCAPS_GLOBALFOCUS         0x00008000
#define DABCAPS_GETCURRENTPOSITION2 0x00010000
#define DABCAPS_MUTE3DATMAXDISTANCE 0x00020000
#define DABCAPS_LOCDEFER            0x00040000
#define DABCAPS_TRUEPLAYPOSITION    0x00080000

#define DABPLAY_LOOPING             0x00000001
#define DABPLAY_LOCHARDWARE         0x00000002
#define DABPLAY_LOCSOFTWARE         0x00000004
#define DABPLAY_TERMINATEBY_TIME    0x00000008
#define DABPLAY_TERMINATEBY_DISTANCE    0x000000010
#define DABPLAY_TERMINATEBY_PRIORITY    0x000000020

#define DABSTATUS_PLAYING           0x00000001
#define DABSTATUS_BUFFERLOST        0x00000002
#define DABSTATUS_LOOPING           0x00000004
#define DABSTATUS_LOCHARDWARE       0x00000008
#define DABSTATUS_LOCSOFTWARE       0x00000010
#define DABSTATUS_TERMINATED        0x00000020

#define DASCL_NORMAL                0x00000001
#define DASCL_PRIORITY              0x00000002
#define DASCL_EXCLUSIVE             0x00000003
#define DASCL_WRITEPRIMARY          0x00000004

#define DASPEAKER_DIRECTOUT         0x00000000
#define DASPEAKER_HEADPHONE         0x00000001
#define DASPEAKER_MONO              0x00000002
#define DASPEAKER_QUAD              0x00000003
#define DASPEAKER_STEREO            0x00000004
#define DASPEAKER_SURROUND          0x00000005
#define DASPEAKER_5POINT1           0x00000006  // obsolete 5.1 setting
#define DASPEAKER_7POINT1           0x00000007  // obsolete 7.1 setting
#define DASPEAKER_7POINT1_SURROUND  0x00000008  // correct 7.1 Home Theater setting
#define DASPEAKER_5POINT1_SURROUND  0x00000009  // correct 5.1 setting
#define DASPEAKER_7POINT1_WIDE      DSSPEAKER_7POINT1
#define DASPEAKER_5POINT1_BACK      DSSPEAKER_5POINT1

#define DASPEAKER_GEOMETRY_MIN      0x00000005  //   5 degrees
#define DASPEAKER_GEOMETRY_NARROW   0x0000000A  //  10 degrees
#define DASPEAKER_GEOMETRY_WIDE     0x00000014  //  20 degrees
#define DASPEAKER_GEOMETRY_MAX      0x000000B4  // 180 degrees

typedef struct _DABUFFERDESC
{
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwBufferBytes;
	DWORD           dwReserved;
	LPWAVEFORMATEX  lpwfxFormat;
	GUID            guid3DAlgorithm;
} DABUFFERDESC, * LPDABUFFERDESC;

typedef struct _DACAPS
{
    DWORD           dwSize;
    DWORD           dwFlags;
    DWORD           dwMinSecondarySampleRate;
    DWORD           dwMaxSecondarySampleRate;
    DWORD           dwPrimaryBuffers;
    DWORD           dwMaxHwMixingAllBuffers;
    DWORD           dwMaxHwMixingStaticBuffers;
    DWORD           dwMaxHwMixingStreamingBuffers;
    DWORD           dwFreeHwMixingAllBuffers;
    DWORD           dwFreeHwMixingStaticBuffers;
    DWORD           dwFreeHwMixingStreamingBuffers;
    DWORD           dwMaxHw3DAllBuffers;
    DWORD           dwMaxHw3DStaticBuffers;
    DWORD           dwMaxHw3DStreamingBuffers;
    DWORD           dwFreeHw3DAllBuffers;
    DWORD           dwFreeHw3DStaticBuffers;
    DWORD           dwFreeHw3DStreamingBuffers;
    DWORD           dwTotalHwMemBytes;
    DWORD           dwFreeHwMemBytes;
    DWORD           dwMaxContigFreeHwMemBytes;
    DWORD           dwUnlockTransferRateHwBuffers;
    DWORD           dwPlayCpuOverheadSwBuffers;
    DWORD           dwReserved1;
    DWORD           dwReserved2;
} DACAPS, * LPDACAPS;

typedef struct _DABCAPS
{
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwBufferBytes;
	DWORD           dwUnlockTransferRate;
	DWORD           dwPlayCpuOverhead;
} DABCAPS, * LPDABCAPS;

#include "daudio_buffer.h"

class IDirectAudio
{
public:
	IDirectAudio(IXAudio2 *XA) : pXA(XA),
		RefVal(0),
		dwSpeakerConf(DASPEAKER_STEREO),
		pPrimary(nullptr)
	{}

	// IUnknown methods
	STDMETHOD(QueryInterface)       (REFIID, LPVOID*);	// 00
	STDMETHOD_(ULONG, AddRef)       ();					// 04
	STDMETHOD_(ULONG, Release)      ();					// 08

	// IDirectSound methods
	STDMETHOD(CreateSoundBuffer)    (const LPDABUFFERDESC pcDABUFFERDESC, IDirectAudioBuffer** ppDSBuffer, LPUNKNOWN pUnkOuter);	// 0C
	STDMETHOD(GetCaps)              (LPDACAPS pDACAPS);																				// 10
	STDMETHOD(DuplicateSoundBuffer) (IDirectAudioBuffer* pDSBufferOriginal, IDirectAudioBuffer** ppDSBufferDuplicate);				// 14
	STDMETHOD(SetCooperativeLevel)  (HWND hwnd, DWORD dwLevel);																		// 18
	STDMETHOD(Compact)              ();																								// 1C
	STDMETHOD(GetSpeakerConfig)     (LPDWORD pdwSpeakerConfig);																		// 20
	STDMETHOD(SetSpeakerConfig)     (DWORD dwSpeakerConfig);																		// 24
	STDMETHOD(Initialize)           (LPCGUID pcGuidDevice);																			// 28

	// IDirectSound8 methods
	STDMETHOD(VerifyCertification)  (LPDWORD pdwCertified);																			// 2C

private:
	u_long RefVal;
	IXAudio2 *pXA;
	DWORD dwSpeakerConf;

	friend class IDirectAudioBuffer;

	IDirectAudioBuffer* pPrimary;
};

HRESULT WINAPI DirectAudioCreate(LPCGUID pcGuidDevice, IDirectAudio** ppDA, LPUNKNOWN pUnkOuter);
