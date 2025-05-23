#include "pch.h"
#include "SDLGamepadManager.h"

#include <exception>

SDLGamepadManager::SDLGamepadManager() {
	SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI, "1");
	SDL_SetHint(SDL_HINT_JOYSTICK_ENHANCED_REPORTS, "auto");
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
		throw new std::exception(SDL_GetError());
	}

	Detect();
}

SDLGamepadManager::~SDLGamepadManager() {
	for (auto gamepad : m_Gamepads) {
		SDL_CloseGamepad(gamepad);
	}
	m_Gamepads.clear();

	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

void SDLGamepadManager::Detect() {
	Sleep(100);
	SDL_UpdateGamepads();

	//
	// Close and reopen all detected gamepads
	//
	for (auto gamepad : m_Gamepads) {
		SDL_CloseGamepad(gamepad);
	}
	m_Gamepads.clear();

	SDL_JoystickID* gamepadIds = SDL_GetGamepads(NULL);
	for (int i = 0; gamepadIds[i]; i++) {
		SDL_Gamepad* gamepad = SDL_OpenGamepad(gamepadIds[i]);
		if (!gamepad) {
			throw new std::exception(SDL_GetError());
		}

		m_Gamepads.push_back(gamepad);
	}

	Sleep(100);
	SDL_UpdateGamepads();
}

SDL_Gamepad* SDLGamepadManager::GetGamepad(size_t i) {
	return m_Gamepads.at(i);
}

size_t SDLGamepadManager::GetGamepadCount() {
	return m_Gamepads.size();
}

void SDLGamepadManager::ExpectButtonState(SDL_GamepadButton Button, bool Down) {
	for (int i = 0; i < 100; i++) {
		SDL_UpdateGamepads();
		if (SDL_GetGamepadButton(GetGamepad(0), Button) == Down) {
			break;
		}
		Sleep(1);
	}

	EXPECT_EQ(SDL_GetGamepadButton(GetGamepad(0), Button), Down) << "Button " << Button << " is not " << (Down ? "down" : "up");
}

void SDLGamepadManager::ExpectHatState(int HatX, int HatY) {
	ExpectButtonState(SDL_GAMEPAD_BUTTON_DPAD_LEFT, HatX < 0);
	ExpectButtonState(SDL_GAMEPAD_BUTTON_DPAD_RIGHT, HatX > 0);
	ExpectButtonState(SDL_GAMEPAD_BUTTON_DPAD_UP, HatY < 0);
	ExpectButtonState(SDL_GAMEPAD_BUTTON_DPAD_DOWN, HatY > 0);
}

void SDLGamepadManager::ExpectAxisValue(SDL_GamepadAxis Axis, Sint16 Value) {
	for (int i = 0; i < 100; i++) {
		SDL_UpdateGamepads();
		if (SDL_GetGamepadAxis(GetGamepad(0), Axis) == Value) {
			break;
		}
		Sleep(1);
	}

	EXPECT_EQ(SDL_GetGamepadAxis(GetGamepad(0), Axis), Value) << "Axis " << Axis << " has unexpected value";
}

void SDLGamepadManager::ExpectTouchpadFingerState(int Finger, bool Down, float TouchX, float TouchY) {
	for (int i = 0; i < 100; i++) {
		SDL_UpdateGamepads();

		bool actualDown = false;
		float actualX = 0;
		float actualY = 0;
		EXPECT_TRUE(SDL_GetGamepadTouchpadFinger(GetGamepad(0), 0, Finger, &actualDown, &actualX, &actualY, NULL));

		if (actualDown == Down &&
			(!Down || (std::fabs(actualX - TouchX) < 0.0001 && std::fabs(actualY - TouchY) < 0.0001))) {
			break;
		}

		Sleep(1);
	}

	bool actualDown;
	float actualX;
	float actualY;
	EXPECT_TRUE(SDL_GetGamepadTouchpadFinger(GetGamepad(0), 0, Finger, &actualDown, &actualX, &actualY, NULL));
	EXPECT_EQ(actualDown, Down) << "Finger " << Finger << " is not " << (Down ? "down" : "up");
	if (Down) {
		EXPECT_FLOAT_EQ(actualX, TouchX) << "Finger " << Finger << " has unexpected X coordinate";
		EXPECT_FLOAT_EQ(actualY, TouchY) << "Finger " << Finger << " has unexpected Y coordinate";
	}
}