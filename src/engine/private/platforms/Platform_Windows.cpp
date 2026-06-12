
#include "Platform.h"
#include "config/ConfigMSWin.h"
#include "window/WinMSWindows.h"
#include "input/InputMSWindows.h"

using namespace zzz::engine;

void Platform::ShutdownPlatformSpecific()
{
	const BOOL result = UnregisterClass(c_RegisterClassName.data(), GetModuleHandle(nullptr));
	if (!result)
	{
		const DWORD error = GetLastError();
		DOutCritical("Failed to unregister window class '{}'. Error code: {}.", c_RegisterClassName.data(), error);
	}
}

void Platform::InitializePlatformSpecific()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	HICON iconHandle = (HICON)LoadImage(
		GetModuleHandle(NULL),
		c_IcoResourceName.data(),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	if (!iconHandle)
		DOutWarning("Failed to load icon '{}'. Error: {}", c_IcoResourceName.data(), GetLastError());

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinMSWindows::WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = c_RegisterClassName.data();
	ATOM Result = RegisterClass(&wc);
	if (Result == 0)
		THROW_RUNTIME("Failed to register window class. Error code: {}.", GetLastError());
}
