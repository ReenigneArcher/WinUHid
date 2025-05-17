#pragma once

#include "WinUHidDevs.h"

typedef struct _WINUHID_XONE_GAMEPAD *PWINUHID_XONE_GAMEPAD;

//
// Creates a new Xbox One gamepad. Device info can optionally be provided to override
// a subset of identification information of the created device.
//
// To destroy the device, call WinUHidXOneDestroy().
//
// On failure, the function will return NULL. Call GetLastError() to the error code.
//
WINUHID_API PWINUHID_XONE_GAMEPAD WinUHidXOneCreate(PCWINUHID_PRESET_DEVICE_INFO Info);

#include <pshpack1.h>

typedef struct _XONE_INPUT_REPORT
{
	USHORT LeftStickX;
	USHORT LeftStickY;
	USHORT RightStickX;
	USHORT RightStickY;
	USHORT LeftTrigger : 10;
	USHORT RightTrigger : 10;
	UCHAR ButtonA : 1;
	UCHAR ButtonB : 1;
	UCHAR ButtonX : 1;
	UCHAR ButtonY : 1;
	UCHAR ButtonLB : 1;
	UCHAR ButtonRB : 1;
	UCHAR ButtonBack : 1;
	UCHAR ButtonMenu : 1;
	UCHAR ButtonLS : 1;
	UCHAR ButtonRS : 1;
	UCHAR Reserved3: 6;
	UCHAR Hat : 4;
	UCHAR Reserved4: 4;
	UCHAR ButtonHome : 1;
	UCHAR Reserved5: 7;
	UCHAR BatteryLevel; // 0 - 0xFF
} XONE_INPUT_REPORT, *PXONE_INPUT_REPORT;
typedef CONST XONE_INPUT_REPORT* PCXONE_INPUT_REPORT;

#include <poppack.h>

//
// Submits an input report to the device.
//
// On failure, the function will return FALSE. Call GetLastError() to the error code.
//
WINUHID_API BOOL WinUHidXOneReportInput(PWINUHID_XONE_GAMEPAD Gamepad, PCXONE_INPUT_REPORT Report);

//
// Destroys the Xbox One gamepad.
//
// This function never fails as long as the provided argument is valid.
//
WINUHID_API VOID WinUHidXOneDestroy(PWINUHID_XONE_GAMEPAD Gamepad);