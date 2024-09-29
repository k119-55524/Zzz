#pragma once

#if defined(_WINDOWS)
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN // Исключите редко используемые компоненты из заголовков Windows

// Файлы заголовков Windows
#include <windows.h>
#endif