#pragma once

#include <format>
#include <string>
#include <cstring>
#include <iostream>
#include <source_location>

#include <zdefines.h>

#if defined(Z_WINDOWS)
#include <Windows.h>
#elif defined(Z_ANDROID)
#include <android/log.h>
#endif