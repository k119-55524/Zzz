#pragma once

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <array>
#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <random>
#include <memory>
#include <sstream>
#include <tchar.h>
#include <assert.h>
#include <string.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <filesystem>
#include <functional>

using namespace std;

#if defined(_DEBUG) && defined(_WINDOWS)
#define DebugOutput(msg) {OutputDebugString(msg);}
#else
#define DebugOutput(msg)
#endif