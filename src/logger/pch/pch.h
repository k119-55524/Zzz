#pragma once

#include <vector>
#include <mutex>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdint>
#include <condition_variable>

#include <core/Core.h>

#if Z_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#elif Z_ANDROID
#include <android/log.h>
#endif
