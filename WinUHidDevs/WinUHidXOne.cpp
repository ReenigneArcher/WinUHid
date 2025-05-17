#include "pch.h"
#include "WinUHidXOne.h"

//
// This device emulates a Microsoft Xbox One gamepad
//

const BYTE k_XOneReportDescriptor[] =
{
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x05,        // Usage (Game Pad)
	0xA1, 0x01,        // Collection (Application)
	0xA1, 0x00,        //   Collection (Physical)
	0x09, 0x30,        //     Usage (X)
	0x09, 0x31,        //     Usage (Y)
	0x15, 0x00,        //     Logical Minimum (0)
	0x27, 0xFF, 0xFF, 0x00, 0x00,  //     Logical Maximum (65534)
	0x95, 0x02,        //     Report Count (2)
	0x75, 0x10,        //     Report Size (16)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection
	0xA1, 0x00,        //   Collection (Physical)
	0x09, 0x33,        //     Usage (Rx)
	0x09, 0x34,        //     Usage (Ry)
	0x15, 0x00,        //     Logical Minimum (0)
	0x27, 0xFF, 0xFF, 0x00, 0x00,  //     Logical Maximum (65534)
	0x95, 0x02,        //     Report Count (2)
	0x75, 0x10,        //     Report Size (16)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection
	0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
	0x09, 0x32,        //   Usage (Z)
	0x15, 0x00,        //   Logical Minimum (0)
	0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
	0x95, 0x01,        //   Report Count (1)
	0x75, 0x0A,        //   Report Size (10)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x00,        //   Logical Maximum (0)
	0x75, 0x06,        //   Report Size (6)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
	0x09, 0x35,        //   Usage (Rz)
	0x15, 0x00,        //   Logical Minimum (0)
	0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
	0x95, 0x01,        //   Report Count (1)
	0x75, 0x0A,        //   Report Size (10)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x00,        //   Logical Maximum (0)
	0x75, 0x06,        //   Report Size (6)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x09,        //   Usage Page (Button)
	0x19, 0x01,        //   Usage Minimum (0x01)
	0x29, 0x0A,        //   Usage Maximum (0x0A)
	0x95, 0x0A,        //   Report Count (10)
	0x75, 0x01,        //   Report Size (1)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x00,        //   Logical Maximum (0)
	0x75, 0x06,        //   Report Size (6)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
	0x09, 0x39,        //   Usage (Hat switch)
	0x15, 0x01,        //   Logical Minimum (1)
	0x25, 0x08,        //   Logical Maximum (8)
	0x35, 0x00,        //   Physical Minimum (0)
	0x46, 0x3B, 0x01,  //   Physical Maximum (315)
	0x66, 0x14, 0x00,  //   Unit (System: English Rotation, Length: Centimeter)
	0x75, 0x04,        //   Report Size (4)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
	0x75, 0x04,        //   Report Size (4)
	0x95, 0x01,        //   Report Count (1)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0x00,        //   Logical Maximum (0)
	0x35, 0x00,        //   Physical Minimum (0)
	0x45, 0x00,        //   Physical Maximum (0)
	0x65, 0x00,        //   Unit (None)
	0x81, 0x03,        //   Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xA1, 0x02,        //   Collection (Logical)
	0x05, 0x0F,        //     Usage Page (PID Page)
	0x09, 0x97,        //     Usage (0x97)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x01,        //     Logical Maximum (1)
	0x75, 0x04,        //     Report Size (4)
	0x95, 0x01,        //     Report Count (1)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x00,        //     Logical Maximum (0)
	0x91, 0x03,        //     Output (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0x70,        //     Usage (0x70)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x64,        //     Logical Maximum (100)
	0x75, 0x08,        //     Report Size (8)
	0x95, 0x04,        //     Report Count (4)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0x50,        //     Usage (0x50)
	0x66, 0x01, 0x10,  //     Unit (System: SI Linear, Time: Seconds)
	0x55, 0x0E,        //     Unit Exponent (-2)
	0x26, 0xFF, 0x00,  //     Logical Maximum (255)
	0x95, 0x01,        //     Report Count (1)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x09, 0xA7,        //     Usage (0xA7)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x65, 0x00,        //     Unit (None)
	0x55, 0x00,        //     Unit Exponent (0)
	0x09, 0x7C,        //     Usage (0x7C)
	0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              //   End Collection
	0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
	0x09, 0x80,        //   Usage (Sys Control)
	0xA1, 0x00,        //   Collection (Physical)
	0x09, 0x85,        //     Usage (Sys Main Menu)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x01,        //     Logical Maximum (1)
	0x95, 0x01,        //     Report Count (1)
	0x75, 0x01,        //     Report Size (1)
	0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x15, 0x00,        //     Logical Minimum (0)
	0x25, 0x00,        //     Logical Maximum (0)
	0x75, 0x07,        //     Report Size (7)
	0x95, 0x01,        //     Report Count (1)
	0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              //   End Collection
	0x05, 0x06,        //   Usage Page (Generic Dev Ctrls)
	0x09, 0x20,        //   Usage (Battery Strength)
	0x15, 0x00,        //   Logical Minimum (0)
	0x26, 0xFF, 0x00,  //   Logical Maximum (255)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              // End Collection
};

const WINUHID_DEVICE_CONFIG k_XOneConfig =
{
	WINUHID_EVENT_WRITE_REPORT,
	0x045e, // Microsoft
	0x02ff, // Xbox One Controller
	0,
	sizeof(k_XOneReportDescriptor),
	k_XOneReportDescriptor,
	{},
	NULL,
	L"HID\\VID_045E&PID_02FF&IG_00\0",
};

typedef struct _WINUHID_XONE_GAMEPAD {
	PWINUHID_DEVICE Device;
} WINUHID_XONE_GAMEPAD, *PWINUHID_XONE_GAMEPAD;

VOID WinUHidXOneCallback(PVOID CallbackContext, PWINUHID_DEVICE Device, PCWINUHID_EVENT Event)
{
	auto gamepad = (PWINUHID_XONE_GAMEPAD)CallbackContext;
	WinUHidCompleteWriteEvent(Device, Event, TRUE);
}

WINUHID_API PWINUHID_XONE_GAMEPAD WinUHidXOneCreate(PCWINUHID_PRESET_DEVICE_INFO Info)
{
	WINUHID_DEVICE_CONFIG config = k_XOneConfig;
	PopulateDeviceInfo(&config, Info);

	if (config.VendorID == 0) {
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	PWINUHID_XONE_GAMEPAD gamepad = (PWINUHID_XONE_GAMEPAD)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*gamepad));
	if (!gamepad) {
		SetLastError(ERROR_OUTOFMEMORY);
		return NULL;
	}

	gamepad->Device = WinUHidCreateDevice(&config);
	if (!gamepad->Device) {
		WinUHidXOneDestroy(gamepad);
		return NULL;
	}

	if (!WinUHidStartDevice(gamepad->Device, WinUHidXOneCallback, gamepad)) {
		WinUHidXOneDestroy(gamepad);
		return NULL;
	}

	return gamepad;
}

WINUHID_API BOOL WinUHidXOneReportInput(PWINUHID_XONE_GAMEPAD Gamepad, PCXONE_INPUT_REPORT Report)
{
	return WinUHidSubmitInputReport(Gamepad->Device, Report, sizeof(*Report));
}

WINUHID_API VOID WinUHidXOneDestroy(PWINUHID_XONE_GAMEPAD Gamepad)
{
	if (!Gamepad) {
		return;
	}

	if (Gamepad->Device) {
		WinUHidDestroyDevice(Gamepad->Device);
	}

	HeapFree(GetProcessHeap(), 0, Gamepad);
}