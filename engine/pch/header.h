#pragma once

#ifdef _DEBUG
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
#include <string>
#include <random>
#include <memory>
#include <codecvt>
#include <fstream>
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

#define THROW_RUNTIME_ERROR(msg) { \
    throw std::runtime_error( \
        std::string(">>>>> --- ERROR that caused the EXCEPTION ---") + \
        "\n+--- " + msg + \
        "\n+--- Function: " + __FUNCTION__ + \
        "\n+--- File: " + __FILE__ + \
        "\n+--- Line: " + std::to_string(__LINE__) \
    ); \
}
