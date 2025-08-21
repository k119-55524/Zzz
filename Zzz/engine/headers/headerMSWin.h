#pragma once

#if defined(_WIN64)
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <wincodec.h>
#include <wrl/client.h> 

#define WIN32_LEAN_AND_MEAN // Исключите редко используемые компоненты из заголовков Windows

#endif