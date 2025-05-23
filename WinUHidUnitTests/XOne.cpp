#include "pch.h"
#include "Utilities.h"

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

	report.ButtonHome = 1;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
	gm.ExpectButtonState(SDL_GAMEPAD_BUTTON_GUIDE, !!report.ButtonHome);

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

//
// The magic values here match SDL3's parsing logic
//
#define SDL_EXPECTED_STICK_VAL(x) ((int)(x) - 0x8000)
#define SDL_EXPECTED_TRIGGER_VAL(x) (((int)(x) * 0x7FFF) / 0x3FF)
TEST(XOne, AxisMapping) {
	auto gamepad = WinUHidXOneCreate(NULL, NULL, NULL);
	ASSERT_TRUE(gamepad) << "Failed to create gamepad";

	SDLGamepadManager gm;
	ASSERT_EQ(gm.GetGamepadCount(), 1) << "Unable to detect gamepad with SDL";

	WINUHID_XONE_INPUT_REPORT report;
	WinUHidXOneInitializeInputReport(&report);

	for (Uint32 value = 0; value <= 0xFFFF; value += 0xFF) {
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

	for (Uint16 value = 0; value <= 0x3FF; value++) {
		report.LeftTrigger = value;
		ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
		gm.ExpectAxisValue(SDL_GAMEPAD_AXIS_LEFT_TRIGGER, SDL_EXPECTED_TRIGGER_VAL(value));
	}

	for (Uint16 value = 0; value <= 0x3FF; value++) {
		report.RightTrigger = value;
		ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);
		gm.ExpectAxisValue(SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, SDL_EXPECTED_TRIGGER_VAL(value));
	}

	WinUHidXOneDestroy(gamepad);
}

#define MAKE_RUMBLE_VALUE(l, r, tl, tr) ((l) << 24 | (r) << 16 | (tl) << 8 | (tr))

TEST(XOne, RumbleEffects) {
	CallbackData<UINT> rumbleState;

	auto gamepad = WinUHidXOneCreate(NULL,
		[](PVOID CallbackContext, UCHAR LeftMotor, UCHAR RightMotor, UCHAR LeftTriggerMotor, UCHAR RightTriggerMotor) {
			((CallbackData<UINT>*)CallbackContext)->Signal(MAKE_RUMBLE_VALUE(LeftMotor, RightMotor, LeftTriggerMotor, RightTriggerMotor));
		}, &rumbleState);
	ASSERT_TRUE(gamepad) << "Failed to create gamepad";

	SDLGamepadManager gm;
	ASSERT_EQ(gm.GetGamepadCount(), 1) << "Unable to detect gamepad with SDL";

	//
	// SDL depends on some input to correlate the XInput slot with the
	// WGI driver to allow the use of trigger rumble.
	//
	WINUHID_XONE_INPUT_REPORT report;
	WinUHidXOneInitializeInputReport(&report);
	report.ButtonLS = 1;
	report.LeftStickX = 127;
	report.LeftStickY = 892;
	ASSERT_EQ(WinUHidXOneReportInput(gamepad, &report), TRUE);

	rumbleState.Quiesce();

	for (Uint32 i = 1; i <= 0xFFFF; i += 655) {
		ASSERT_TRUE(SDL_RumbleGamepad(gm.GetGamepad(0), i, 0, 100));
		EXPECT_EQ(rumbleState.Wait(), MAKE_RUMBLE_VALUE(i / 655, 0, 0, 0));
	}

	for (Uint32 i = 1; i <= 0xFFFF; i += 655) {
		ASSERT_TRUE(SDL_RumbleGamepad(gm.GetGamepad(0), 0, i, 100));
		EXPECT_EQ(rumbleState.Wait(), MAKE_RUMBLE_VALUE(0, i / 655, 0, 0));
	}

	for (Uint32 i = 1; i <= 0xFFFF; i += 655) {
		ASSERT_TRUE(SDL_RumbleGamepadTriggers(gm.GetGamepad(0), i, 0, 100));
		EXPECT_EQ(rumbleState.Wait(), MAKE_RUMBLE_VALUE(0, 0, i / 655, 0));
	}

	for (Uint32 i = 1; i <= 0xFFFF; i += 655) {
		ASSERT_TRUE(SDL_RumbleGamepadTriggers(gm.GetGamepad(0), 0, i, 100));
		EXPECT_EQ(rumbleState.Wait(), MAKE_RUMBLE_VALUE(0, 0, 0, i / 655));
	}

	WinUHidXOneDestroy(gamepad);
}