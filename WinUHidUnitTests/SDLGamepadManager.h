#pragma once

#include <SDL3/SDL.h>

#include <vector>

class SDLGamepadManager
{
public:
	SDLGamepadManager();
	~SDLGamepadManager();

	void Detect();

	size_t GetGamepadCount();
	SDL_Gamepad* GetGamepad(size_t i);

	void ExpectButtonState(SDL_GamepadButton Button, bool Down);
	void ExpectHatState(int HatX, int HatY);
	void ExpectAxisValue(SDL_GamepadAxis Axis, Sint16 Value);
	void ExpectTouchpadFingerState(int Finger, bool Down, float TouchX, float TouchY);

private:
	std::vector<SDL_Gamepad*> m_Gamepads;
};

