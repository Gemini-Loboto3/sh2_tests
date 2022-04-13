#pragma once

typedef struct MW
{
	struct MW_vtbl
	{
		void* pad[3];
		void (*mwDsClosePort)(MW* mw);
		void (*mwDsPlay)(MW* mw);
		void (*mwDsStop)(MW* mv);
		int  (*mwDsGetBufSize)(MW* mw);
		void (*mwDsSetNumChan)(MW* mw, int chans);
		int  (*mwDsGetPlayPos)(MW* mw);
		void (*mwDsSetSfreq)(MW* mw, int freq);
		int  (*mwDsGetSfreq)(MW* mw);
		void (*mwDsSetBitPerSmpl)(MW* a1, int bits);
		int  (*mwDsGetBitPerSmpl)(MW* a1);
		void (*mwDsSetVol)(MW* mw, int vol);
		int  (*mwDsGetVol)(MW*);
		void (*mwDsSetPan)(MW*, int);
		int  (*mwDsGetPan)(MW*);
		void (*mwDsSetExPrm)(MW* mw);
		void (*mwDsGetExPrm)(MW* a1, int a2, DWORD* a3, DWORD* a4);
		void (*mwDsTrns)(MW* a1, int a2, int a3, void* src, u_long pos);
		void (*mwDsStopTrns)(MW* mw);
		int  (*mwDsGetTrnsStat)(MW* mw);
		int  (*mwDsAdjstBufPause)(MW* mw, int, DWORD*);
		int  (*mwDsAdjstBufDscrd)(MW* mw);
	}* vtbl;

	int used;
	XAudio2Buffer* obj;
	WAVEFORMATEX fmt;
	short* snd_ptr;		// adx sends audio packets here?
	int adx_chans;
	int stereo_out;
	int stopped;
} MW;

typedef struct LibHn
{
	void* pad[3];
	void (*mwDsInit)(IXAudio2* obj);
	void (*mwDsFinish)();
	MW* (*mwDsOpenPort)(u_long cmd);
} LibHn;

extern LibHn XAudioHn_vtbl;
extern MW::MW_vtbl XAudioMW_vtbl;

void mwPcInit();
LibHn* mwSndGetLibHn();
