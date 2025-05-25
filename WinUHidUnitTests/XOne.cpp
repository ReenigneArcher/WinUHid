#include "pch.h"
#include "Utilities.h"

#include <Xinput.h>

TEST(XOne, CreateBasic) {
	auto gamepad = WinUHidXOneCreate(NULL, NULL, NULL);
	ASSERT_TRUE(gamepad) << "Failed to create gamepad";

	SDLGamepadManager gm;
	ASSERT_EQ(gm.GetGamepadCount(), 1) << "Unable to detect gamepad with SDL";

	EXPECT_EQ(SDL_GetGamepadVendor(gm.GetGamepad(0)), 0x045e); // Microsoft
	EXPECT_EQ(SDL_GetGamepadProduct(gm.GetGamepad(0)), 0x02ff); // Xbox One Controller
	EXPECT_EQ(SDL_GetGamepadProductVersion(gm.GetGamepad(0)), 0);

	WinUHidXOneDestroy(gamepad);
}

TEST(XOne, ButtonMapping) {
	auto gamepad = WinUHidXOneCreate(NULL, NULL, NULL);
	ASSERT_TRUE(gamepad) << "Failed to create gamepad";

	SDLGamepadManager gm;
	ASSERT_EQ(gm.GetGamepadCount(), 1) << "Unable to detect gamepad with SDL";

	WINUHID_XONE_INPUT_REPORT report;
	WinUHidXOneInitializeInputReport(&report);

	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			WinUHidXOneSetHatState(&report, x, y);
			ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
			gm.ExpectHatState(x, y);
		}
	}

	report.ButtonA = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_SOUTH, !!report.ButtonA);

	report.ButtonB = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_EAST, !!report.ButtonB);

	report.ButtonY = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_NORTH, !!report.ButtonY);

	report.ButtonX = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_WEST, !!report.ButtonX);

	report.ButtonBack = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_BACK, !!report.ButtonBack);

	report.ButtonMenu = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_START, !!report.ButtonMenu);

#if 0
	//
	// This will steal focus if the Game Bar is not disabled
	//
	report.ButtonHome = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_GUIDE, !!report.ButtonHome);
#endif

	report.ButtonLB = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, !!report.ButtonLB);

	report.ButtonRB = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, !!report.ButtonRB);

	report.ButtonLS = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_LEFT_STICK, !!report.ButtonLS);

	report.ButtonRS = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_RIGHT_STICK, !!report.ButtonRS);

	WinUHidXOneDestroy(gamepad);
}

#define SDL_EXPECTED_STICK_VAL(x) ((int)(x) - 0x8000)
TEST(XOne, AxisMapping) {
	auto gamepad = WinUHidXOneCreate(NULL, NULL, NULL);
	ASSERT_TRUE(gamepad) << "Failed to create gamepad";

	SDLGamepadManager gm;
	ASSERT_EQ(gm.GetGamepadCount(), 1) << "Unable to detect gamepad with SDL";

	WINUHID_XONE_INPUT_REPORT report;
	WinUHidXOneInitializeInputReport(&report);

	for (Uint32 value = 0; value <= 0xFFFF; value += 0xFF) {
		if (value == 0xFF) {
			//
			// FIXME: Figure out why SDL returns -32768 for a raw left stick axis value of FF
			//
			continue;
		}
		report.LeftStickX = (Uint16)value;
		ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
		gm.ExpectAxisValue(SDL_GAMEPAD_AXIS_LEFTX, SDL_EXPECTED_STICK_VAL(value));
	}

	for (Uint32 value = 0; value <= 0xFFFF; value += 0xFF) {
		report.LeftStickY = (Uint16)value;
		ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
		gm.ExpectAxisValue(SDL_GAMEPAD_AXIS_LEFTY, SDL_EXPECTED_STICK_VAL(value));
	}

	for (Uint32 value = 0; value <= 0xFFFF; value += 0xFF) {
		report.RightStickX = (Uint16)value;
		ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
		gm.ExpectAxisValue(SDL_GAMEPAD_AXIS_RIGHTX, SDL_EXPECTED_STICK_VAL(value));
	}

	for (Uint32 value = 0; value <= 0xFFFF; value += 0xFF) {
		report.RightStickY = (Uint16)value;
		ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
		gm.ExpectAxisValue(SDL_GAMEPAD_AXIS_RIGHTY, SDL_EXPECTED_STICK_VAL(value));
	}

	//
	// We read the XInput API directly to avoid trigger scaling weirdness with SDL
	//

	for (Uint16 value = 0; value <= 0xFF; value++) {
		report.LeftTrigger = value * 0x3FF / 0xFF;
		ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
		Sleep(10);
		XINPUT_STATE state;
		XInputGetState(0, &state);
		EXPECT_EQ(state.Gamepad.bLeftTrigger, value);
	}

	for (Uint16 value = 0; value <= 0xFF; value++) {
		report.RightTrigger = value * 0x3FF / 0xFF;
		ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
		Sleep(10);
		XINPUT_STATE state;
		XInputGetState(0, &state);
		EXPECT_EQ(state.Gamepad.bRightTrigger, value);
	}

	WinUHidXOneDestroy(gamepad);
}

#define MAKE_RUMBLE_VALUE(l, r, tl, tr) ((l) << 24 | (r) << 16 | (tl) << 8 | (tr))

TEST(XOne, RumbleEffects) {
	CallbackData<UINT> rumbleState;

	//
	// Rumble only works reliably for testing under GameInput
	//
	SDL_SetHint(SDL_HINT_JOYSTICK_GAMEINPUT, "1");

	auto gamepad = WinUHidXOneCreate(NULL,
		[](PVOID CallbackContext, UCHAR LeftMotor, UCHAR RightMotor, UCHAR LeftTriggerMotor, UCHAR RightTriggerMotor) {
			((CallbackData<UINT>*)CallbackContext)->Signal(MAKE_RUMBLE_VALUE(LeftMotor, RightMotor, LeftTriggerMotor, RightTriggerMotor));
		}, &rumbleState);
	ASSERT_TRUE(gamepad) << "Failed to create gamepad";

	//
	// GameInput requires a window for focus tracking, so tell SDLGamepadManager to create one
	//
	SDLGamepadManager gm(true);
	ASSERT_EQ(gm.GetGamepadCount(), 1) << "Unable to detect gamepad with SDL";

	ASSERT_TRUE(SDL_RumbleGamepad(gm.GetGamepad(0), 32768, 0, 100));
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(50, 0, 0, 0));

	ASSERT_TRUE(SDL_RumbleGamepad(gm.GetGamepad(0), 65535, 0, 100));
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(100, 0, 0, 0));

	ASSERT_TRUE(SDL_RumbleGamepad(gm.GetGamepad(0), 0, 32768, 100));
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(0, 50, 0, 0));

	ASSERT_TRUE(SDL_RumbleGamepad(gm.GetGamepad(0), 0, 65535, 100));
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(0, 100, 0, 0));

	//
	// Rumble will cease after 100ms
	//
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(0, 0, 0, 0));

	ASSERT_TRUE(SDL_RumbleGamepadTriggers(gm.GetGamepad(0), 32768, 0, 100));
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(0, 0, 50, 0));

	ASSERT_TRUE(SDL_RumbleGamepadTriggers(gm.GetGamepad(0), 65535, 0, 100));
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(0, 0, 100, 0));

	ASSERT_TRUE(SDL_RumbleGamepadTriggers(gm.GetGamepad(0), 0, 32768, 100));
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(0, 0, 0, 50));

	ASSERT_TRUE(SDL_RumbleGamepadTriggers(gm.GetGamepad(0), 0, 65535, 100));
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(0, 0, 0, 100));

	//
	// Rumble will cease after 100ms
	//
	EXPECT_CB_VALUE(rumbleState, MAKE_RUMBLE_VALUE(0, 0, 0, 0));

	WinUHidXOneDestroy(gamepad);

	SDL_SetHint(SDL_HINT_JOYSTICK_GAMEINPUT, "0");
}