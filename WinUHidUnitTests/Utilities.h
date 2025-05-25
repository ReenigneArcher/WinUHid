#pragma once

#include <SDL3/SDL.h>

#include <atomic>
#include <vector>

#include <wrl/wrappers/corewrappers.h>

class SDLGamepadManager
{
public:
	explicit SDLGamepadManager(bool NeedsWindow = false);
	~SDLGamepadManager();

	void Detect();

	size_t GetGamepadCount();
	SDL_Gamepad* GetGamepad(size_t i);

	void ExpectButtonState(SDL_GamepadButton Button, bool Down);
	void ExpectHatState(int HatX, int HatY);
	void ExpectAxisValue(SDL_GamepadAxis Axis, Sint16 Value);
	void ExpectTouchpadFingerState(int Finger, bool Down, float TouchX, float TouchY);

private:
	static int EventPollThread(void* ptr);

	std::vector<SDL_Gamepad*> m_Gamepads;
	SDL_Window* m_Window;
	SDL_Thread* m_PollingThread;
	std::atomic<bool> m_StopThread;
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

	T Wait(T ExpectedData) {
		do {
			if (WaitForSingleObject(m_Event.Get(), 1000) != WAIT_OBJECT_0) {
				return T();
			}
		} while (m_Data != ExpectedData);

		return m_Data;
	}

private:
	Microsoft::WRL::Wrappers::Event m_Event;
	std::atomic<T> m_Data;
};

#define EXPECT_CB_VALUE(state, value) EXPECT_EQ((state).Wait(value), value)
