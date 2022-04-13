/*
* =====================================================
* Main XAudio2 module and injections
* =====================================================
*/
#include "audio.h"
#include "..\inject.h"

HMODULE xdll = nullptr;
HRESULT(__stdcall* XAudio2CreateEx)(_Outptr_ IXAudio2** ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor);

// XAudio 2.9 dll loader
int Xaudio2_loader()
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

IXAudio2 *pXAudio2;
IXAudio2MasteringVoice* pMasterVoice;

// replaces DirectSoundCreate8
HRESULT __stdcall Xaudio2_init(int, void **obj, int)
{
	Xaudio2_loader();

	XAudio2CreateEx(&pXAudio2, 0, 0);
	pXAudio2->CreateMasteringVoice(&pMasterVoice, 2, 44100);
	*obj = pXAudio2;

	return S_OK;
}

// Xaudio2 injections
void Inject_xaudio2()
{
	INJECT(0x55A3C6, Xaudio2_init);

	memset((void*)0x514F6F, 0x90, 7);	// nop cooperative mode for dsound

	//INJECT(0x56C650, mwPcInit);
	*(DWORD*)0x56CA5F = (DWORD)&XAudioMW_vtbl;
	INJECT(0x56D360, mwSndGetLibHn);

	INJECT(0x517820, Sound_Create3dBuffer);
	INJECT(0x518860, Sound_CreateBuffer);

	INJECT(0x43D5B0, BinkQuery);
}

// --------------------------------
// Main object
void XAudio2Buffer::Create(IXAudio2* xa, LPWAVEFORMATEX fmt)
{
	ZeroMemory(&buffer, sizeof(buffer));
	buffer.pContext = this;
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = XA_BUFFER_SIZE;
	buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	buffer.LoopBegin = 0;
	buffer.LoopLength = XA_BUFFER_SIZE / fmt->nBlockAlign;

	xa->CreateSourceVoice(&pVoice, fmt, 0, 1.f, &cb);

	sent = 0;
	freq = (float)fmt->nSamplesPerSec;

	is_locked = 0;
}

void XAudio2Buffer::Release()
{
	if (pVoice)
	{
		pVoice->DestroyVoice();
		pVoice = nullptr;
	}
}

void XAudio2Buffer::SendBuffer(short* in)
{
	memcpy(data, in, XA_BUFFER_SIZE);

	buffer.pAudioData = data;
	pVoice->SubmitSourceBuffer(&buffer);

	sent++;
}

void XAudio2Buffer::Play()
{
	if (pVoice)
	{
		if (sent == 0)
		{
			buffer.pAudioData = (BYTE*)data;
			pVoice->SubmitSourceBuffer(&buffer);
			sent++;
		}
		pVoice->Start();
	}
}

void XAudio2Buffer::Stop()
{
	if (pVoice)
		pVoice->Stop();
}

void XAudio2Buffer::SetFrequency(u_long new_freq)
{
	if (pVoice)
		pVoice->SetFrequencyRatio((float)new_freq / freq);
}

void XAudio2Buffer::Rewind(u_long pos)
{
	if (pVoice)
	{
		pVoice->FlushSourceBuffers();
		//pVoice->SubmitSourceBuffer(&buffer);
	}
}

BYTE* XAudio2Buffer::Lock(u_long pos, u_long size)
{
	if (is_locked)
		return nullptr;
	if (size == 0) return nullptr;

	lock_pos = pos;
	lock_size = size;
	lock_buf = &data[pos];
	is_locked = 1;

	return lock_buf;
}

void XAudio2Buffer::Unlock()
{
	if (is_locked)
		is_locked = 0;
}

// --------------------------------
// Main object callbacks
void BGMCallback::OnBufferEnd(void* pBufferContext)
{
	XAudio2Buffer* b = (XAudio2Buffer*)pBufferContext;
}