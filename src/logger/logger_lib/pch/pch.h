#pragma once

#include <format>
#include <string>
#include <cstring>
#include <iostream>
#include <source_location>

#include <common/platform_defines.h>

#if Z_WINDOWS
#include <Windows.h>
#elif Z_ANDROID
#include <android/log.h>
#endif