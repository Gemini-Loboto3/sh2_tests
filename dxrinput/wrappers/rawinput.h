#pragma once
#include "..\..\framework.h"
#include <Windows.h>
#include <WinUser.h>
#include <vector>
#include <string>
#include <hidsdi.h>

class CRawDevice;

class CRawInput
{
public:
	CRawInput();
	~CRawInput();

	void Create(HWND hWnd);
	void Release();
	void GetInputData(LPARAM lParam, WPARAM wParam);
	void DetectRemoval(LPARAM lParam, WPARAM wParam);
	int  Select(int sel);

	int Detection(HANDLE hDevice, DWORD dwType);

	HWND hWnd;
	std::vector<CRawDevice*> device;
	CRawDevice *cur_device;
	DWORD dwNumJoysticks;
	int selected;
	std::vector<u8> buffer;
};

class CRawDevice
{
public:
	CRawDevice(HANDLE device, DWORD type)
	{
		hDevice = device; dwType = type;
		hHidDevice = INVALID_HANDLE_VALUE;
		ZeroMemory(bButtonStates, sizeof(bButtonStates));
		lAxisX = 0x8000;
		lAxisY = 0x8000;
		lAxisZ = 0x8000;
		lAxisRx = 0x8000;
		lAxisRy = 0x8000;
		lAxisRz = 0x8000;
		lHat = 8;
		pValueCaps = NULL;
		pButtonCaps = NULL;
		pPreparsedData = NULL;
		g_NumberOfButtons = 0;
		ds4_open = false;
		GetDeviceInfo();
	}

	~CRawDevice()
	{
		if(pPreparsedData) free(pPreparsedData);
		if(pButtonCaps)    free(pButtonCaps);
		if(pValueCaps)     free(pValueCaps);

		if (ds4_open)
		{
			Ds4Rumble(0, 0);
			Ds4Led(0, 0, 0);
			Ds4Update();
		}

		if (hHidDevice != INVALID_HANDLE_VALUE)
			CloseHandle(hHidDevice);
	}

	void GetDeviceInfo();
	void GetDeviceStrings();
	char* GetDeviceName();
	void  GetDeviceInfo(RID_DEVICE_INFO *pInfo);
	std::string GetDeviceNameStd();

	void ParseRawInput(PRAWHID pRawInput);
	void Invalidate();
	std::string GetEnumerator();

	int IsDs4();
	__inline bool Ds4IsOpen() { return ds4_open == 1 || ds4_open == 3; }
	__inline bool Ds3IsOpen() { return ds4_open == 2; }
	void Ds4Rumble(u8 left, u8 right);
	void Ds4Led(u8 r, u8 g, u8 b);
	void Ds4Update();

	typedef struct tagDs4Buffer
	{
		u8 header[2];	// 0x05 0x0ff
		u8 unk[2];
		u8 leftMotor,
			rightMotor;
		u8 ledR,
			ledG,
			ledB;
		u8 pad[23];
	} DS4_BUFFER;
#include <pshpack1.h>
	typedef struct tagDs5Trigger
	{
		u8 type,
			params[10];
	} DS5_TRIGGER;

	typedef struct tagDs5Buffer
	{
		u8 master;		// 0
		u16 flags;		// 1
		u8 rightMotor,	// 2
			leftMotor;	// 3
		u8 unk4[4];		// 4-5-6-7
		u8 mic_led;		// 8 0 off, 1 on, 2 pulse
		u8 unk9;		// 9
		DS5_TRIGGER RightTrg,	// 10
			LeftTrg;	// 21
		u8 unk28[11];	// 32
		u8 player_led;	// 44 no idea
		u8 ledR,		// 45
			ledG,		// 46
			ledB;		// 47
	} DS5_BUFFER;
#include <poppack.h>

	HANDLE hDevice;
	DWORD dwType;
	UINT g_NumberOfButtons;
	DWORD dwAxis, dwHat;
	PHIDP_PREPARSED_DATA pPreparsedData;
	HIDP_CAPS Caps;
	PHIDP_VALUE_CAPS pValueCaps;
	PHIDP_BUTTON_CAPS pButtonCaps;
	BOOL bButtonStates[128];

	std::string name, manufacturer;
	GUID guid;
	HANDLE hHidDevice;

	DS4_BUFFER ds4_buf;
	DS5_BUFFER ds5_buf;
	int ds4_open;
	size_t report_size;

	LONG lAxisX, lAxisY, lAxisZ;
	LONG lAxisRx, lAxisRy, lAxisRz;
	LONG lHat;
};

