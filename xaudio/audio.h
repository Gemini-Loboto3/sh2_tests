#pragma once

#include "..\framework.h"
#include "include/xaudio2redist.h"

#define XA_BUFFER_SIZE	(16384)

extern IXAudio2* pXAudio2;

// simple callback override for signaling
class BGMCallback : public IXAudio2VoiceCallback
{
public:
	BGMCallback() :
		hEndEvent(CreateEventA(nullptr, FALSE, FALSE, nullptr))
	{
	}
	~BGMCallback()
	{
		CloseHandle(hEndEvent);
	}

	virtual void _stdcall OnStreamEnd() {}
	virtual void _stdcall OnVoiceProcessingPassEnd() {}
	virtual void _stdcall OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
	virtual void _stdcall OnBufferEnd(void* pBufferContext);
	virtual void _stdcall OnBufferStart(void* pBufferContext) {}
	virtual void _stdcall OnLoopEnd(void* pBufferContext) {}
	virtual void _stdcall OnVoiceError(void* pBufferContext, HRESULT Error) {}

	HANDLE hEndEvent;
};

class XAudio2Buffer
{
public:
	void Create(IXAudio2* xa, LPWAVEFORMATEX fmt);
	void Release();

	IXAudio2SourceVoice* pVoice;
	XAUDIO2_BUFFER buffer;
	BGMCallback cb;

	__inline u_long GetBufSize() { return XA_BUFFER_SIZE; }

	__inline BYTE* GetBuffer() { return data; }
	void SendBuffer(short* in);

	BYTE* Lock(u_long pos, u_long size);
	void Unlock();

	void Play();
	void Stop();
	void SetFrequency(u_long new_freq);
	void Rewind(u_long pos);

	BYTE data[XA_BUFFER_SIZE];
	u_long sent;
	float freq;
	// lock info
	BYTE* lock_buf;
	u_long lock_pos,
		lock_size,
		is_locked;
};

#include "audio_adx.h"
#include "audio_game.h"
