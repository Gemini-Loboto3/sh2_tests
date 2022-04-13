#include "..\..\framework.h"
#include <joystickapi.h>
#include <setupapi.h>
#include "RawInput.h"

#pragma warning(disable: 4996)
#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")

CRawInput in;

void Inject_rawinput()
{
	in.Create(NULL);
}

CRawInput::CRawInput()
{
}


CRawInput::~CRawInput()
{
	Release();
}

void CRawInput::Create(HWND hWnd)
{
	cur_device = NULL;

	RAWINPUTDEVICE Rid[2];

	// controllers
	Rid[0].hwndTarget = hWnd;
	Rid[0].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
	Rid[0].usUsagePage = 1;
	Rid[0].usUsage = 5;
	// joysticks
	Rid[1].hwndTarget = hWnd;
	Rid[1].dwFlags = RIDEV_INPUTSINK | RIDEV_DEVNOTIFY;
	Rid[1].usUsagePage = 1;
	Rid[1].usUsage = 4;
	if (!RegisterRawInputDevices(Rid, 2, sizeof(Rid[0])))
	{
		dwNumJoysticks = 0;
		return;
	}

	UINT numDevs;
	GetRawInputDeviceList(NULL, &numDevs, sizeof(RAWINPUTDEVICELIST));
	RAWINPUTDEVICELIST *List = new RAWINPUTDEVICELIST[numDevs];
	GetRawInputDeviceList(List, &numDevs, sizeof(RAWINPUTDEVICELIST));

	dwNumJoysticks = 0;
	for (UINT i = 0; i < numDevs; i++)
	{
		if (List[i].dwType != RIM_TYPEHID) continue;

		CRawDevice *dev = new CRawDevice(List[i].hDevice, List[i].dwType);
		if (dev->g_NumberOfButtons >= 6 && (dev->dwAxis >= 2 || dev->dwHat))
		{
			device.push_back(dev);
			dwNumJoysticks++;
		}
		else delete dev;
	}
	delete[] List;

	this->hWnd = hWnd;
}

void CRawInput::Release()
{
	RAWINPUTDEVICE Rid[2];
	// controllers
	Rid[0].hwndTarget = hWnd;
	Rid[0].dwFlags = 0;
	Rid[0].usUsagePage = 1;
	Rid[0].usUsage = 5;
	// joysticks
	Rid[1].hwndTarget = hWnd;
	Rid[1].dwFlags = 0;
	Rid[1].usUsagePage = 1;
	Rid[1].usUsage = 4;

	RegisterRawInputDevices(Rid, 2, sizeof(Rid[0]));
	for (size_t i = 0, si = device.size(); i < si; i++)
		delete device[i];
	device.clear();

	cur_device = NULL;
}

int CRawInput::Detection(HANDLE hDevice, DWORD dwType)
{
	if (dwType != RIM_TYPEHID) return -1;

	CRawDevice *dev = new CRawDevice(hDevice, dwType);
	for (size_t i = 0, si = device.size(); i < si; i++)
	{
		// already exists, ignore
		if (device[i]->hDevice == hDevice)
			return -1;
		// probably the same device, try replacing this shit
		if (device[i]->name == dev->name && device[i]->guid == dev->guid)
		{
			delete device[i];
			device[i] = dev;
			return i;
		}
	}
	delete dev;
	return -1;
}

void CRawInput::GetInputData(LPARAM lParam, WPARAM wParam)
{
	if (cur_device == NULL)
		return;

	UINT size = 0;

	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
	// accomodate buffer resizing
	if(buffer.size() < size)
		buffer = std::vector<u8>(size);
	GetRawInputData((HRAWINPUT)lParam, RID_INPUT, buffer.data(), &size, sizeof(RAWINPUTHEADER));
	RAWINPUT *in = (RAWINPUT*)buffer.data();

	if (in->header.hDevice == cur_device->hDevice)
		cur_device->ParseRawInput(&in->data.hid);
}

int CRawInput::Select(int sel)
{
	if (dwNumJoysticks == 0)
	{
		cur_device = NULL;
		return -1;
	}

	if (sel >= (int)dwNumJoysticks)
	{
		cur_device = device[0];
		return 0;
	}

	if (sel == -1) sel = 0;	// fix unselected controller issue
	cur_device = device[sel];
	return sel;
}

//////////////////////////////////////////////
void CRawDevice::GetDeviceInfo()
{
	GetDeviceStrings();

	// get caps
	UINT bufferSize;
	GetRawInputDeviceInfoA(hDevice, RIDI_PREPARSEDDATA, NULL, &bufferSize);
	pPreparsedData = (_HIDP_PREPARSED_DATA*)malloc(bufferSize);
	GetRawInputDeviceInfoA(hDevice, RIDI_PREPARSEDDATA, pPreparsedData, &bufferSize);

	Caps;
	HidP_GetCaps(pPreparsedData, &Caps);
	report_size = Caps.OutputReportByteLength;
	pButtonCaps = (PHIDP_BUTTON_CAPS)malloc(sizeof(HIDP_BUTTON_CAPS) * Caps.NumberInputButtonCaps);

	USHORT capsLength = Caps.NumberInputButtonCaps;
	HidP_GetButtonCaps(HidP_Input, pButtonCaps, &capsLength, pPreparsedData);
	g_NumberOfButtons = pButtonCaps->Range.UsageMax - pButtonCaps->Range.UsageMin + 1;

	pValueCaps = (PHIDP_VALUE_CAPS)malloc(sizeof(HIDP_VALUE_CAPS) * Caps.NumberInputValueCaps);
	capsLength = Caps.NumberInputValueCaps;
	HidP_GetValueCaps(HidP_Input, pValueCaps, &capsLength, pPreparsedData);

	dwAxis = 0;
	dwHat = 0;
	for (size_t i = 0, si = capsLength; i < si; i++)
	{
		switch (pValueCaps[i].Range.UsageMin)
		{
		case HID_USAGE_GENERIC_X:
		case HID_USAGE_GENERIC_Y:
		case HID_USAGE_GENERIC_Z:
		case HID_USAGE_GENERIC_RX:
		case HID_USAGE_GENERIC_RY:
		case HID_USAGE_GENERIC_RZ:
			dwAxis++;
		case HID_USAGE_GENERIC_HATSWITCH:
			dwHat++;
			break;
		}
	}
}

std::string WideToMulti_s(const wchar_t *multi);

char* CRawDevice::GetDeviceName()
{
	UINT bufferSize;
	GetRawInputDeviceInfoA(hDevice, RIDI_DEVICENAME, NULL, &bufferSize);
	char *__name = new char[bufferSize];
	GetRawInputDeviceInfoA(hDevice, RIDI_DEVICENAME, __name, &bufferSize);

	return __name;
}

void CRawDevice::GetDeviceInfo(RID_DEVICE_INFO *pInfo)
{
	UINT bufferSize = sizeof(*pInfo);
	memset(pInfo, 0, sizeof(*pInfo));
	pInfo->cbSize = sizeof(RID_DEVICE_INFO);
	pInfo->dwType = RIM_TYPEHID;
	GetRawInputDeviceInfoA(hDevice, RIDI_DEVICEINFO, pInfo, &bufferSize);
}

std::string CRawDevice::GetDeviceNameStd()
{
	char *n = GetDeviceName();
	std::string str = std::string(n);
	delete[] n;

	return str;
}

std::string CRawDevice::GetEnumerator()
{
	std::string n = std::string("HID");

	char *__name = GetDeviceName();

	if (IsDs4())
	{
		hHidDevice = CreateFileA(__name, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, 0, NULL);

		memset(&ds4_buf, 0, sizeof(ds4_buf));
		ds4_buf.header[0] = 0x05;
		ds4_buf.header[1] = 0xFF;

		memset(&ds5_buf, 0, sizeof(ds5_buf));
		ds5_buf.master = 2;
		ds5_buf.flags = 0xf7ff;
	}

	char *t = strtok(__name, "#");
	for (int i = 0; t; i++)
	{
		if (i > 0 && i < 3)
		{
			n += "\\";
			n += t;
		}

		t = strtok(NULL, "#");
	}
	delete[] __name;


	return n;
}

void CRawDevice::GetDeviceStrings()
{
	SP_DEVINFO_DATA devInfoData;
	SP_DEVICE_INTERFACE_DATA devIntData;

	HidD_GetHidGuid(&guid);
	HDEVINFO hDevInfo = SetupDiGetClassDevsA(&guid, GetEnumerator().c_str(), NULL, DIGCF_PRESENT | DIGCF_INTERFACEDEVICE);
	
	devInfoData.cbSize = sizeof(devInfoData);
	devIntData.cbSize = sizeof(devIntData);
	DWORD MemberIndex = 0;
	while (1)
	{
		if (!SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &guid, MemberIndex, &devIntData))
		{
			SetupDiDestroyDeviceInfoList(hDevInfo);
			break;
		}
		MemberIndex++;

		char devbuffer[256 + 4];
		SP_DEVICE_INTERFACE_DETAIL_DATA_A *detailData = (SP_DEVICE_INTERFACE_DETAIL_DATA_A*)devbuffer;
		detailData->cbSize = 5;
		DWORD Reguired = 0;
		if (!SetupDiGetDeviceInterfaceDetailA(hDevInfo, &devIntData, detailData, 256, &Reguired, NULL))
			continue;

		HANDLE DeviceHandle = CreateFileA(detailData->DevicePath,
			GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			0,
			NULL);

		if (DeviceHandle != INVALID_HANDLE_VALUE)
		{
			wchar_t _name[256];
			if (name.size() == 0 && HidD_GetProductString(DeviceHandle, _name, sizeof(_name)))
				name = WideToMulti_s(_name);
			if (HidD_GetManufacturerString(DeviceHandle, _name, sizeof(_name)))
				manufacturer = WideToMulti_s(_name);
		}
		CloseHandle(DeviceHandle);
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);
}

#define DS5_PRODUCT_ID		0xCE6

#define DS4_PRODUCT_ID		0x5C4
#define DS4_2_PRODUCT_ID	0x9CC
#define DS4_W_PRODUCT_ID	0xBA0
#define DS4_VENDOR_ID		0x54C

#define DS3_VENDOR_ID		0x54C
#define DS3_PRODUCT_ID		0x268

int CRawDevice::IsDs4()
{
	RID_DEVICE_INFO info;
	GetDeviceInfo(&info);

	if (info.hid.dwVendorId == DS4_VENDOR_ID)
	{
		switch (info.hid.dwProductId)
		{
		case DS4_PRODUCT_ID:
			name = std::string("Sony DualShock®4");
			ds4_open = 1;
			break;
		case DS4_2_PRODUCT_ID:
			name = std::string("Sony DualShock®4 (V2)");
			ds4_open = 1;
			break;
		case DS4_W_PRODUCT_ID:
			name = std::string("Sony DualShock®4 (Wireless Adaptor)");
			ds4_open = 1;
			break;
		case DS5_PRODUCT_ID:
			name = std::string("Sony DualSense®");
			ds4_open = 3;
			break;
		case DS3_PRODUCT_ID:
			name = std::string("Sony DualShock®3");
			ds4_open = 2;
			break;
		default:
			ds4_open = 0;
		}
	}
	else ds4_open = 0;

	return ds4_open;
}

void CRawDevice::Ds4Rumble(u8 left, u8 right)
{
	ds4_buf.leftMotor = left;
	ds4_buf.rightMotor = right;

	ds5_buf.leftMotor = left;
	ds5_buf.rightMotor = right;
}

void CRawDevice::Ds4Led(u8 r, u8 g, u8 b)
{
	ds4_buf.ledR = r;
	ds4_buf.ledG = g;
	ds4_buf.ledB = b;

	ds5_buf.ledR = r;
	ds5_buf.ledG = g;
	ds5_buf.ledB = b;
}

void CRawDevice::Ds4Update()
{
	DWORD written;
	if (ds4_open == 1)
	{
		if(report_size == sizeof(ds4_buf))
			WriteFile(hHidDevice, &ds4_buf, sizeof(ds4_buf), &written, NULL);
	}
	else
	{
		if(report_size == sizeof(ds5_buf))
			WriteFile(hHidDevice, &ds5_buf, sizeof(ds5_buf), &written, NULL);
	}
}

LONG ConvertAxis(HIDP_VALUE_CAPS *caps, ULONG value)
{
	value <<= (16 - caps->BitSize);
	return value & 0xffff;
}

static void setBit(u32 &bits, USAGE index)
{
	if (index >= HID_USAGE_GENERIC_X && index <= HID_USAGE_GENERIC_HATSWITCH)
		bits |= 1 << (index - HID_USAGE_GENERIC_X);
}

static bool getBit(u32 bits, USAGE index)
{
	if (index >= HID_USAGE_GENERIC_X && index <= HID_USAGE_GENERIC_HATSWITCH)
		return bits &  1 << (index - HID_USAGE_GENERIC_X) ? true : false;
	return true;
}

void CRawDevice::ParseRawInput(PRAWHID pRawInput)
{
	USAGE usage[128];
	ULONG usageLength = g_NumberOfButtons;
	HidP_GetUsages(HidP_Input, pButtonCaps->UsagePage, 0, usage, &usageLength, pPreparsedData, (PCHAR)pRawInput->bRawData, pRawInput->dwSizeHid);

	ZeroMemory(bButtonStates, sizeof(bButtonStates));
	for (ULONG i = 0; i < usageLength; i++)
		bButtonStates[usage[i] - pButtonCaps->Range.UsageMin] = TRUE;

	u32 bits = 0;
	for (USHORT i = 0; i < Caps.NumberInputValueCaps; i++)
	{
		ULONG value;
		HidP_GetUsageValue(HidP_Input, pValueCaps[i].UsagePage, 0, pValueCaps[i].Range.UsageMin, &value, pPreparsedData,
				(PCHAR)pRawInput->bRawData, pRawInput->dwSizeHid);

		if (getBit(bits, pValueCaps[i].Range.UsageMin))
			continue;

		switch (pValueCaps[i].Range.UsageMin)
		{
		case HID_USAGE_GENERIC_X:	// X-axis
			lAxisX = ConvertAxis(&pValueCaps[i], value);
			break;
		case HID_USAGE_GENERIC_Y:	// Y-axis
			lAxisY = ConvertAxis(&pValueCaps[i], value);
			break;
		case HID_USAGE_GENERIC_Z:	// Z-axis
			lAxisZ = ConvertAxis(&pValueCaps[i], value);
			break;
		case HID_USAGE_GENERIC_RX: // Rotate-X
			lAxisRx = ConvertAxis(&pValueCaps[i], value);
			break;
		case HID_USAGE_GENERIC_RY: // Rotate-Y
			lAxisRy = ConvertAxis(&pValueCaps[i], value);
			break;
		case HID_USAGE_GENERIC_RZ: // Rotate-Z
			lAxisRz = ConvertAxis(&pValueCaps[i], value);
			break;
		case HID_USAGE_GENERIC_HATSWITCH:	// Hat Switch
			if ((pValueCaps[i].LogicalMin == 0 && pValueCaps[i].LogicalMax == 7))
				lHat = value;
			if (pValueCaps[i].LogicalMin == 1 && pValueCaps[i].LogicalMax == 8)
			{
				if (value <= 0) lHat = 8;
				else lHat = value - 1;
			}
			break;
		}
		setBit(bits, pValueCaps[i].Range.UsageMin);
	}
}

void CRawDevice::Invalidate()
{
}
