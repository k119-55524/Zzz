#include <foundation.h>

#include "Platform.h"
#include "window/WinMSWindows.h"
#include "input/platforms/InputMSWindows.h"
#include "../core/config/platforms/ConfigMSWin.h"

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
					DefWindowProc(hWnd, uMsg, wParam, lParam);
				else
					DefWindowProc(hWnd, uMsg, wParam, lParam);
			}

			return procRes.result;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

void Platform::ShutdownPlatformSpecific()
{
	const BOOL result = UnregisterClass(static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetClassName().c_str(), GetModuleHandle(nullptr));
	if (!result)
	{
		const DWORD error = GetLastError();
		DOutCritical("Failed to unregister window class '{}'. Error code: {}.", static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetClassName(), error);
	}
}

void Platform::InitializePlatformSpecific()
{
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	HICON iconHandle = (HICON)LoadImage(
		GetModuleHandle(NULL),
		static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetIcoResourceName().c_str(),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	if (!iconHandle)
		DOutWarning("Failed to load icon '{}'. Error: {}", static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetIcoResourceName(), GetLastError());

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = static_cast<const ConfigMSWin&>(m_ConfigManager->GetPlatformConfig()).GetClassName().c_str();
	ATOM Result = RegisterClass(&wc);
	if (Result == 0)
		THROW_RUNTIME("Failed to register window class. Error code: {}.", GetLastError());
}
