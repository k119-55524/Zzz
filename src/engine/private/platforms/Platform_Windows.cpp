
#include "Platform.h"
#include "config/ConfigMSWin.h"
#include "window/WinMSWindows.h"
#include "input/InputMSWindows.h"

using namespace zzz::engine;

namespace
{
	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
	{
		WinMSWindows::MSWinCtx* ctx = nullptr;

		if (uMsg == WM_NCCREATE)
		{
			const auto* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
			ctx = static_cast<WinMSWindows::MSWinCtx*>(pCreate->lpCreateParams);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ctx));
		}
		else
			ctx = reinterpret_cast<WinMSWindows::MSWinCtx*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		
		if (ctx)
		{
			static bool IsHandleInput = true;
			if (uMsg == WM_CLOSE)
				IsHandleInput = false;

			auto procRes = ctx->window->MsgProc(hWnd, uMsg, wParam, lParam);
			if (procRes.isContinue)
			{
				if (IsHandleInput && ctx->input->ProcessMessage({ uMsg, wParam, lParam }))
					return 0;

				return DefWindowProc(hWnd, uMsg, wParam, lParam);
			}

			return procRes.result;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

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
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = c_RegisterClassName.data();
	ATOM Result = RegisterClass(&wc);
	if (Result == 0)
		THROW_RUNTIME("Failed to register window class. Error code: {}.", GetLastError());
}
