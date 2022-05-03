#include <windows.h>
#include "daudio.h"

IDirectAudioBuffer::IDirectAudioBuffer(IDirectAudio* DA, LPDABUFFERDESC ddesc) :
	fmt{ 0 }
{
	desc = *ddesc;
	pDA = DA;

	is_primary = desc.dwFlags & DABCAPS_PRIMARYBUFFER ? true : false;
	if (is_primary)
	{
		pDA->pXA->CreateMasteringVoice(&Voice.mv, 2, 44100);
		pDA->pPrimary = this;
	}
	else
	{
		pDA->pXA->CreateSourceVoice(&Voice.sv, desc.lpwfxFormat);
		buffer = new BYTE[desc.dwBufferBytes];
		dwBytes = desc.dwBufferBytes;
	}

	if (desc.lpwfxFormat != nullptr)
	{
		fmt = *desc.lpwfxFormat;
		desc.lpwfxFormat = &fmt;
	}

	pListener = new IDirectAudio3DListener;
}

// IUnknown methods
HRESULT IDirectAudioBuffer::QueryInterface(REFIID refid, LPVOID* ppo)
{
	if (refid == IID_IDirectSound3DListener)
	{
		*ppo = pListener;
	}

	return S_OK;
}

ULONG IDirectAudioBuffer::AddRef()
{
	return ++pDA->RefVal;
}

ULONG IDirectAudioBuffer::Release()
{
	ULONG ret = --pDA->RefVal;
	delete this;
	return ret;
}

// IDirectSoundBuffer methods
HRESULT IDirectAudioBuffer::GetCaps(LPDABCAPS pDSBufferCaps)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::GetCurrentPosition(LPDWORD pdwCurrentPlayCursor, LPDWORD pdwCurrentWriteCursor)
{
	XAUDIO2_VOICE_STATE st;
	Voice.sv->GetState(&st);

	if (pdwCurrentPlayCursor) *pdwCurrentPlayCursor = (st.SamplesPlayed * fmt.nBlockAlign) % dwBytes;

	return S_OK;
}
HRESULT IDirectAudioBuffer::GetFormat(LPWAVEFORMATEX pwfxFormat, DWORD dwSizeAllocated, LPDWORD pdwSizeWritten)
{
	*pwfxFormat = fmt;

	return S_OK;
}
HRESULT IDirectAudioBuffer::GetVolume(LPLONG plVolume)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::GetPan(LPLONG plPan)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::GetFrequency(LPDWORD pdwFrequency)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::GetStatus(LPDWORD pdwStatus)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::Initialize(IDirectAudio* pDirectSound, const LPDABUFFERDESC pcDSBufferDesc)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::Lock(DWORD dwOffset, DWORD dwBytes, LPVOID* ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID* ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
{
	if(ppvAudioPtr1) *ppvAudioPtr1 = &buffer[dwOffset];
	if(pdwAudioBytes1) *pdwAudioBytes1 = dwBytes;

	if(ppvAudioPtr2) *ppvAudioPtr2 = nullptr;
	if(pdwAudioBytes2) *pdwAudioBytes2 = 0;

	return S_OK;
}

HRESULT IDirectAudioBuffer::Play(DWORD dwReserved1, DWORD dwPriority, DWORD dwFlags)
{
	XAUDIO2_BUFFER b = { 0 };
	b.AudioBytes = dwBytes;
	b.PlayLength = dwBytes / fmt.nBlockAlign;
	b.pAudioData = buffer;
	if (dwFlags & DABPLAY_LOOPING)
		b.LoopCount = XAUDIO2_LOOP_INFINITE;

	Voice.sv->SubmitSourceBuffer(&b);
	Voice.sv->Start();

	return S_OK;
}
HRESULT IDirectAudioBuffer::SetCurrentPosition(DWORD dwNewPosition)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::SetFormat(LPCWAVEFORMATEX pcfxFormat)
{
	fmt = *pcfxFormat;
	if (is_primary)
	{
		//Voice.mv->DestroyVoice();
		//pDA->pXA->CreateMasteringVoice(&Voice.mv, 2, fmt.nChannels, fmt.nSamplesPerSec);
	}

	return S_OK;
}
HRESULT IDirectAudioBuffer::SetVolume(LONG lVolume)
{
	volume = lVolume;
	float v = XAudio2DecibelsToAmplitudeRatio(lVolume / 100.f);
	Voice.sv->SetVolume(v);

	return S_OK;
}
HRESULT IDirectAudioBuffer::SetPan(LONG lPan)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::SetFrequency(DWORD dwFrequency)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::Stop()
{
	Voice.sv->Stop();

	return S_OK;
}
HRESULT IDirectAudioBuffer::Unlock(LPVOID pvAudioPtr1, DWORD dwAudioBytes1, LPVOID pvAudioPtr2, DWORD dwAudioBytes2)
{
	return S_OK;
}
HRESULT IDirectAudioBuffer::Restore()
{
	return S_OK;
}