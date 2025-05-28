//
// pch.h
//

#pragma once

#include "gtest/gtest.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "../WinUHidDevs/WinUHidMouse.h"
#include "../WinUHidDevs/WinUHidXOne.h"
#include "../WinUHidDevs/WinUHidPS4.h"
#include "../WinUHidDevs/WinUHidPS5.h"
#include "../WinUHid/WinUHid.h"

#include <SDL3/SDL_hidapi.h>
