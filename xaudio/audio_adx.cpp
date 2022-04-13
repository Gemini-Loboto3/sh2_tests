/*
* =====================================================
* ADX module for XAudio2
* =====================================================
*/
#include "audio.h"
#include <stdio.h>
#include <intrin.h>

#define MAX_MW		16

static MW mwg_xa_obj[MAX_MW];
static IXAudio2* mwg_xa_sound;
static XAudio2Buffer* mwg_xa_buffer;

static void print_caller(void *ptr, const char *func_name)
{
	char str[32];
	sprintf_s(str, sizeof(str), "%08X:%s\n", (u_long)ptr, func_name);
	OutputDebugStringA(str);
}

// writes stereo
static void mwlDsCopy(void* dst, void* src, size_t size)
{
	memcpy(dst, src, size);
}

// writes mono as stereo
static void mwlDsCopy1(void* dst, void* src, size_t size)
{
	short* d16 = (short*)dst;
	short* s16 = (short*)src;

	for (size_t i = 0; i < size / 4; i++, d16 += 2, s16++)
		d16[0] = d16[1] = *s16;
}

// writes one stereo channel
static void mwlDsCopy2(void* dst, void* src, size_t size)
{
	short* d16 = (short*)dst;
	short* s16 = (short*)src;

	for (size_t i = 0; i < size / 4; i++, s16++, d16 += 2)
		*d16 = *s16;
}

#define CALL_LOG		print_caller(_ReturnAddress(), __FUNCTION__);

static MW* mwlDsAlloc()
{
	int found = -1;

	for (int i = 0; i < MAX_MW; i++)
	{
		if (mwg_xa_obj[i].used == 0)
		{
			found = i;
			break;
		}
	}

	if (found == -1) return nullptr;

	return &mwg_xa_obj[found];
}

static void mwlDsFree(MW* mw)
{
	mw->used = 0;
}

void mwlDsMakeDmyWfx(u_short channels, LPWAVEFORMATEX fmt)
{
	ZeroMemory(fmt, sizeof(*fmt));

	fmt->nChannels = channels;
	fmt->nBlockAlign = 2 * channels;
	fmt->wFormatTag = WAVE_FORMAT_PCM;
	fmt->nSamplesPerSec = 44100;
	fmt->wBitsPerSample = 16;
	fmt->cbSize = sizeof(*fmt);
	fmt->nAvgBytesPerSec = 44100 * 2 * channels;
}

static XAudio2Buffer* mwlDsSetupBuf(u_short chans, u_long size)
{
	WAVEFORMATEX fmt;
	mwlDsMakeDmyWfx(chans, &fmt);

	XAudio2Buffer* buf = new XAudio2Buffer();
	buf->Create(pXAudio2, &fmt);
	return buf;
}

static void mwPcInit()
{
	WAVEFORMATEX fmt;
	mwlDsMakeDmyWfx(1, &fmt);

	mwg_xa_buffer->Create(pXAudio2, &fmt);
	memset(mwg_xa_buffer->data, 0, mwg_xa_buffer->GetBufSize());
}

//
void mwDsInit(IXAudio2 *obj)
{
	mwg_xa_sound = obj;
	for (int i = 0; i < MAX_MW; i++)
	{
		mwg_xa_obj[i].used = 0;
		mwg_xa_obj[i].obj = nullptr;
	}
}

void mwDsFinish()
{

}

MW* mwDsOpenPort(u_long cmd)
{
	if (cmd < 1 || cmd > 2)
		return nullptr;

	auto mw = mwlDsAlloc();
	if (mw)
	{
		mw->vtbl = &XAudioMW_vtbl;
		mw->used = 1;
		mw->obj = mwlDsSetupBuf(cmd & 0xffff, cmd << 15);
		mw->adx_chans = cmd;
		mw->stereo_out = cmd;
		mwlDsMakeDmyWfx(cmd & 0xffff, &mw->fmt);

		return mw;
	}

	return nullptr;
}

LibHn XAudioHn_vtbl =
{
	nullptr, nullptr, nullptr,
	mwDsInit, mwDsFinish, mwDsOpenPort
};

LibHn* mwSndGetLibHn()
{
	return &XAudioHn_vtbl;
}

//.data:008C16CC
void mwDsClosePort(MW* mw)
{
	CALL_LOG;
	if (mw->used)
	{
		if (mw->obj)
		{
			mw->obj->Stop();
			mw->obj->Release();
			delete mw->obj;
			mw->obj = nullptr;
		}
		mwlDsFree(mw);
	}
}

//.data:008C16D0
void mwDsPlay(MW* mw)
{
	if (mw->used && mw->obj)
	{
		if (mw->stopped == 1)
			mw->stopped = 0;

		mw->obj->SetFrequency(mw->fmt.nSamplesPerSec);
		mw->obj->Rewind(0);
		mw->obj->Play();
	}
}
//.data:008C16D4
void mwDsStop(MW* mw)
{
	CALL_LOG;

	if (mw->used && mw->obj)
	{
		mw->obj->Stop();
	}
}
//.data:008C16D8
// return work buffer size in samples
int mwDsGetBufSize(MW* mw)
{
	mw->snd_ptr = (short*)mw->obj->GetBuffer();

	return mw->obj->GetBufSize() / mw->fmt.nBlockAlign;
}
//.data:008C16DC
void mwDsSetNumChan(MW* mw, int chans)
{
	mw->stereo_out = chans;
}
//.data:008C16E0
// gets position of the buffer in samples
int mwDsGetPlayPos(MW* mw)
{
	if (mw->used && mw->obj)
	{
		if (mw->stopped == 1)
			return 0;
		XAUDIO2_VOICE_STATE state;
		mw->obj->pVoice->GetState(&state);
		return state.SamplesPlayed % (mw->obj->GetBufSize() / mw->fmt.nBlockAlign);
	}

	return 0;
}
//.data:008C16E4
void mwDsSetSfreq(MW* mw, int freq)
{
	if (mw->used && mw->obj)
	{
		mw->fmt.nSamplesPerSec = freq;
		mw->obj->SetFrequency(freq);
	}
}
//.data:008C16E8
int mwDsGetSfreq(MW* mw)
{
	return mw->fmt.nSamplesPerSec;
}
//.data:008C16EC
void mwDsSetBitPerSmpl(MW* mw, int bits)
{
	mw->fmt.wBitsPerSample = bits;
}
//.data:008C16F0
int mwDsGetBitPerSmpl(MW* mw)
{
	return mw->fmt.wBitsPerSample;
}
//.data:008C16F4
void mwDsSetVol(MW* mw, int vol)
{
	char mes[64];
	sprintf_s(mes, sizeof(mes), "%s: vol %d\n", __FUNCTION__, vol);
	OutputDebugStringA(mes);

	if (mw->used && mw->obj)
	{
		float volume;
		if (vol == -1000) volume = 0.f;
		else volume = 1.f - (vol / -1000.f);
		mw->obj->pVoice->SetVolume(volume);
	}
}
//.data:008C16F8
// unused
int mwDsGetVol(MW*)
{
	return 0;
}
//.data:008C16FC
// unused
void mwDsSetPan(MW*, int pan)
{
}
//.data:008C1700
// unused
int mwDsGetPan(MW* mw)
{
	return 0;
}
//.data:008C1704
// empty
void mwDsSetExPrm(MW* mw)
{
}
//.data:008C1708
// useless
void mwDsGetExPrm(MW* mw, int a2, DWORD* a3, DWORD* a4)
{
	*a3 = 0;
	*a4 = 0;
}

//.data:008C170C
// sends audio samples
void mwDsTrns(MW* mw, int is_right, int pos /*this is in samples*/, void* src, u_long samples)
{
	if (mw->used && mw->obj)
	{
		size_t size = samples * mw->fmt.nBlockAlign;	// samples to bytes
		if (size >= 64)
		{
			BYTE *buf = mw->obj->Lock(pos * mw->fmt.nBlockAlign, size);

			// normal
			if (mw->adx_chans == 1)
				mwlDsCopy(buf, src, size);
			// mono to stereo
			else if (mw->stereo_out == 1)
				mwlDsCopy1(buf, src, size);
			// interleaved stereo
			else
				mwlDsCopy2(buf + (is_right ? 2 : 0), src, size);

			mw->obj->Unlock();
		}
	}
}
//.data:008C1710
void mwDsStopTrns(MW* mw)
{
	mw->stopped = 1;
}
//.data:008C1714
// empty
int mwDsGetTrnsStat(MW* mw)
{
	return 1;
}
//.data:008C1718
int mwDsAdjstBufPause(MW* mw, int, DWORD* a3)
{
	*a3 = 0;
	return 0;
}
//.data:008C171C
// empty
int mwDsAdjstBufDscrd(MW* mw)
{
	return 0;
}

MW::MW_vtbl XAudioMW_vtbl =
{
	nullptr, nullptr, nullptr,
	mwDsClosePort,
	mwDsPlay,
	mwDsStop,
	mwDsGetBufSize,
	mwDsSetNumChan,
	mwDsGetPlayPos,
	mwDsSetSfreq,
	mwDsGetSfreq,
	mwDsSetBitPerSmpl,
	mwDsGetBitPerSmpl,
	mwDsSetVol,
	mwDsGetVol,
	mwDsSetPan,
	mwDsGetPan,
	mwDsSetExPrm,
	mwDsGetExPrm,
	mwDsTrns,
	mwDsStopTrns,
	mwDsGetTrnsStat,
	mwDsAdjstBufPause,
	mwDsAdjstBufDscrd
};
