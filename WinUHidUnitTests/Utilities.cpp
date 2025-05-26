#include "pch.h"
#include "Utilities.h"

#include <exception>

int SDLGamepadManager::EventPollThread(void* ptr) {
	auto gm = (SDLGamepadManager*)ptr;

	while (!gm->m_StopThread) {
		SDL_Event event;
		if (!SDL_WaitEventTimeout(&event, 1)) {
			continue;
		}

		if (event.type == SDL_EVENT_QUIT) {
			ExitProcess(-1);
		}
	}

	return 0;
}

SDLGamepadManager::SDLGamepadManager(bool NeedsWindow) {
	SDL_SetHint(SDL_HINT_JOYSTICK_ENHANCED_REPORTS, "auto");
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

	if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO)) {
		throw new std::exception(SDL_GetError());
	}

	if (NeedsWindow) {
		//
		// We need a window that can take focus in order for some gamepad APIs to work
		//
		m_Window = SDL_CreateWindow("Gamepad Unit Test", 800, 600, 0);
	}
	else {
		m_Window = NULL;
	}

	//
	// Pump the event loop in a separate thread to keep the SDL joystick subsystem happy
	//
	m_PollingThread = SDL_CreateThread(EventPollThread, "EventPollThread", this);

	Detect();
}

SDLGamepadManager::~SDLGamepadManager() {
	m_StopThread = 1;
	SDL_WaitThread(m_PollingThread, NULL);

	for (auto gamepad : m_Gamepads) {
		SDL_CloseGamepad(gamepad);
	}
	m_Gamepads.clear();

	SDL_DestroyWindow(m_Window);

	SDL_QuitSubSystem(SDL_INIT_GAMEPAD | SDL_INIT_VIDEO);
}

void SDLGamepadManager::Detect() {
	SDL_Delay(100);

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

	SDL_Delay(100);
}

SDL_Gamepad* SDLGamepadManager::GetGamepad(size_t i) {
	return m_Gamepads.at(i);
}

size_t SDLGamepadManager::GetGamepadCount() {
	return m_Gamepads.size();
}

void SDLGamepadManager::ExpectButtonState(SDL_GamepadButton Button, bool Down) {
	for (int i = 0; i < 100; i++) {
		if (SDL_GetGamepadButton(GetGamepad(0), Button) == Down) {
			break;
		}
		SDL_Delay(1);
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
		if (SDL_GetGamepadAxis(GetGamepad(0), Axis) == Value) {
			break;
		}
		SDL_Delay(1);
	}

	EXPECT_EQ(SDL_GetGamepadAxis(GetGamepad(0), Axis), Value) << "Axis " << Axis << " has unexpected value";
}

void SDLGamepadManager::ExpectTouchpadFingerState(int Finger, bool Down, float TouchX, float TouchY) {
	for (int i = 0; i < 100; i++) {
		bool actualDown = false;
		float actualX = 0;
		float actualY = 0;
		EXPECT_TRUE(SDL_GetGamepadTouchpadFinger(GetGamepad(0), 0, Finger, &actualDown, &actualX, &actualY, NULL));

		if (actualDown == Down &&
			(!Down || (std::fabs(actualX - TouchX) < 0.0001 && std::fabs(actualY - TouchY) < 0.0001))) {
			break;
		}
		SDL_Delay(1);
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

void SDLGamepadManager::ExpectSensorData(SDL_SensorType sensor, float* values) {
	for (int i = 0; i < 100; i++) {
		float actualValues[3];
		EXPECT_TRUE(SDL_GetGamepadSensorData(GetGamepad(0), sensor, actualValues, ARRAYSIZE(actualValues)));

		if (std::fabs(actualValues[0] - values[0]) < 0.01 &&
			std::fabs(actualValues[1] - values[1]) < 0.01 &&
			std::fabs(actualValues[2] - values[2]) < 0.01) {
			break;
		}
		SDL_Delay(1);
	}

	float actualValues[3];
	EXPECT_TRUE(SDL_GetGamepadSensorData(GetGamepad(0), sensor, actualValues, ARRAYSIZE(actualValues)));
	ASSERT_NEAR(actualValues[0], values[0], 0.01) << "Sensor " << sensor << "has unexpected X coordinate";
	ASSERT_NEAR(actualValues[1], values[1], 0.01) << "Sensor " << sensor << "has unexpected Y coordinate";
	ASSERT_NEAR(actualValues[2], values[2], 0.01) << "Sensor " << sensor << "has unexpected Z coordinate";
}