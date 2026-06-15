#pragma once

#include <vector>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdint>
#include <condition_variable>

#include <common/common.h>

#if Z_WINDOWS
#include <Windows.h>
#elif Z_ANDROID
#include <android/log.h>
#endif