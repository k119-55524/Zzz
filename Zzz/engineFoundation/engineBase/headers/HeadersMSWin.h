#pragma once

#pragma once

#if defined(ZPLATFORM_MSWINDOWS)

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN 

#include <windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <hidsdi.h>
#include <hidusage.h>

#endif // ZPLATFORM_MSWINDOWS