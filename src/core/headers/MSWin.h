#pragma once

#if Z_WINDOWS
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <windowsx.h>
#include <winioctl.h>
#include <iphlpapi.h>
#include <comdef.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <hidsdi.h>
#include <hidusage.h>

#pragma comment(lib, "IPHLPAPI.lib")
#endif // Z_WINDOWS