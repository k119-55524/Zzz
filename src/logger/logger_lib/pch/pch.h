#pragma once

#include <format>
#include <string>
#include <cstring>
#include <iostream>
#include <source_location>

#if defined(_MSC_VER)
#include <Windows.h>
#elif defined(__ANDROID__)
#include <android/log.h>
#endif