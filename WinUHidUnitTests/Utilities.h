#pragma once

#include <SDL3/SDL.h>

#include <vector>

#include <wrl/wrappers/corewrappers.h>

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

template <typename T>
class CallbackData
{
public:
	CallbackData() : m_Data()
	{
		m_Event.Attach(CreateEventW(NULL, FALSE, FALSE, NULL));
	}

	void Signal(T Data) {
		m_Data = Data;
		SetEvent(m_Event.Get());
	}

	void Quiesce() {
		while (WaitForSingleObject(m_Event.Get(), 1000) == WAIT_OBJECT_0);
	}

	T Wait() {
		if (WaitForSingleObject(m_Event.Get(), 10000) != WAIT_OBJECT_0) {
			return T();
		}

		return m_Data;
	}

private:
	Microsoft::WRL::Wrappers::Event m_Event;
	T m_Data;
};
