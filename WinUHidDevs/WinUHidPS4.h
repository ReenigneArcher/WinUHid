#pragma once

#include "WinUHidDevs.h"

typedef struct _WINUHID_PS4_GAMEPAD *PWINUHID_PS4_GAMEPAD;

#include <pshpack1.h>

typedef struct _WINUHID_PS4_INPUT_REPORT
{
	UCHAR ReportId; // Do not modify

	UCHAR LeftStickX; // 0x80 is centered
	UCHAR LeftStickY; // 0x80 is centered
	UCHAR RightStickX; // 0x80 is centered
	UCHAR RightStickY; // 0x80 is centered

	UCHAR Hat : 4;
	UCHAR ButtonSquare : 1;
	UCHAR ButtonCross : 1;
	UCHAR ButtonCircle : 1;
	UCHAR ButtonTriangle : 1;
	UCHAR ButtonL1 : 1;
	UCHAR ButtonR1 : 1;
	UCHAR ButtonL2 : 1;
	UCHAR ButtonR2 : 1;
	UCHAR ButtonShare : 1;
	UCHAR ButtonOptions : 1;
	UCHAR ButtonL3 : 1;
	UCHAR ButtonR3 : 1;
	UCHAR ButtonHome : 1;
	UCHAR ButtonTouchpad : 1;
	UCHAR Reserved : 6;

	UCHAR LeftTrigger;
	UCHAR RightTrigger;

	USHORT Timestamp; // Calculated by WinUHid
	UCHAR BatteryLevel; // 0 to 0xFF

	USHORT GyroX;
	USHORT GyroY;
	USHORT GyroZ;
	USHORT AccelX;
	USHORT AccelY;
	USHORT AccelZ;
	UCHAR Reserved2[5];
	BYTE BatteryLevelSpecial;

	UCHAR Status[2];

	//
	// Use WinUHidPS4SetTouchReport() to set these fields
	//
	UCHAR TouchReportCount;
	struct {
		UCHAR Timestamp;
		struct {
			UCHAR ContactSeq;
			UCHAR XLowPart;
			UCHAR XHighPart : 4;
			UCHAR YLowPart : 4;
			UCHAR YHighPart;
		} TouchPoints[2];
	} TouchReports[3];

	UCHAR Reserved3[3];
} WINUHID_PS4_INPUT_REPORT, * PWINUHID_PS4_INPUT_REPORT;
typedef CONST WINUHID_PS4_INPUT_REPORT* PCWINUHID_PS4_INPUT_REPORT;

#include <poppack.h>

//
// Optional callback to be invoked when rumble motor state changes
//
typedef VOID WINUHID_PS4_FF_CB(PVOID CallbackContext, UCHAR LeftMotor, UCHAR RightMotor);
typedef WINUHID_PS4_FF_CB* PWINUHID_PS4_FF_CB;

//
// Optional callback to be invoked when LED state changes
//
typedef VOID WINUHID_PS4_LED_CB(PVOID CallbackContext, UCHAR LedRed, UCHAR LedGreen, UCHAR LedBlue);
typedef WINUHID_PS4_LED_CB* PWINUHID_PS4_LED_CB;

typedef struct _WINUHID_PS4_GAMEPAD_INFO {
	//
	// Basic HID and PnP device information (optional)
	//
	PCWINUHID_PRESET_DEVICE_INFO BasicInfo;

	//
	// Unique identifier for this PS4 gamepad
	//
	UCHAR MacAddress[6];
} WINUHID_PS4_GAMEPAD_INFO, *PWINUHID_PS4_GAMEPAD_INFO;
typedef CONST WINUHID_PS4_GAMEPAD_INFO* PCWINUHID_PS4_GAMEPAD_INFO;

//
// Creates a new PS4 gamepad.
//
// To destroy the device, call WinUHidPS4Destroy().
//
// On failure, the function will return NULL. Call GetLastError() to the error code.
//
WINUHID_API PWINUHID_PS4_GAMEPAD WinUHidPS4Create(PCWINUHID_PS4_GAMEPAD_INFO Info, PWINUHID_PS4_FF_CB RumbleCallback, PWINUHID_PS4_LED_CB LedCallback, PVOID CallbackContext);

//
// Initializes the input report with neutral data.
//
WINUHID_API VOID WinUHidPS4InitializeInputReport(PWINUHID_PS4_INPUT_REPORT Report);

//
// Sets the battery state in the input report.
//
WINUHID_API VOID WinUHidPS4SetBatteryState(PWINUHID_PS4_INPUT_REPORT Report, BOOL Wired, UCHAR Percentage);

//
// Sets the touch state in the input report.
//
// The PS4 controller supports 2 simultaneous touches (TouchIndex 0 and 1).
// The touchpad is 1920x943, so TouchX/Y must be within those dimensions.
//
WINUHID_API VOID WinUHidPS4SetTouchReport(PWINUHID_PS4_INPUT_REPORT Report, UCHAR TouchIndex, BOOL TouchDown, USHORT TouchX, USHORT TouchY);

//
// Submits an input report to the device.
//
// On failure, the function will return FALSE. Call GetLastError() to the error code.
//
WINUHID_API BOOL WinUHidPS4ReportInput(PWINUHID_PS4_GAMEPAD Gamepad, PCWINUHID_PS4_INPUT_REPORT Report);

//
// Destroys the gamepad.
//
// This function never fails as long as the provided argument is valid.
//
WINUHID_API VOID WinUHidPS4Destroy(PWINUHID_PS4_GAMEPAD Gamepad);