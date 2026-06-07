#if defined(Z_WINDOWS)

#include "WinMSWindows.h"
#include "../../platforms/PlatformMSWindows.h"
#include "../platforms/native_view/ScreenResolution.h"

using namespace zzz::engine;

WinMSWindows::WinMSWindows(const std::shared_ptr<IPlatform> platform) :
	IWindow(platform),
	m_hWnd(nullptr)
{
}

WinMSWindows::~WinMSWindows()
{
}

[[nodiscard]] std::expected<void, std::string> WinMSWindows::Initialize(const std::string_view appName)
{
	std::shared_ptr<PlatformMSWindows> platform = std::dynamic_pointer_cast<PlatformMSWindows>(m_Platform);
	ensure(platform != nullptr, "Platform is not PlatformLinux.");

	// Рассчитать размеры прямоугольника окна на основе запрошенных размеров клиентской области.
	Size2D<LONG> winSize;
	// TODO: не правильная архитектура. Подумать как задавать размер окна
	winSize.SetFrom(platform->GetWinSize());
	RECT R = { 0, 0, winSize.width, winSize.height };
	AdjustWindowRectEx(&R, WS_OVERLAPPEDWINDOW, false, 0);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
	int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
	int xPos = (screenWidth - width) / 2;  // Расчет позиции по оси X
	int yPos = (screenHeight - height) / 2; // Расчет позиции по оси Y
	CreateWindowEx(
		0,
		platform->GetPlatformConfig().GetClassName().c_str(),
		appName.data(),
		WS_OVERLAPPEDWINDOW,
		xPos, yPos, width, height,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		this);

	if (!m_hWnd)
		THROW_RUNTIME("CreateWindowEx( ... ) failed. Error code (Windows): {}", ::GetLastError());

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return std::expected<void, std::string>();
}

LRESULT WinMSWindows::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
		m_hWnd = hWnd;
		break;

		//case WM_CREATE:
		//	return InitRawInput();

	case WM_CLOSE:
		onCloseRequested();
		DestroyWindow(hWnd);
		return 0;

	// Engine сам считает к-во открытых окон
	//case WM_DESTROY:
	//	break;

	// Обрабатываем изменение размера окна
	case WM_SIZE:
	{
		m_WinSize.SetFrom(static_cast<zU32>(LOWORD(lParam)), static_cast<zU32>(HIWORD(lParam)));
		DOut("WM_SIZE {}: {}.", static_cast<void*>(m_hWnd), m_WinSize.ToString());
	//	if (wParam == SIZE_MINIMIZED)
	//	{
	//		OnResize(m_WinSize, eTypeWinResize::Hide);
	//		IsMinimized = true;
	//	}
	//	else
	//	{
	//		if ((wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) && IsMinimized)
	//		{
	//			OnResize(m_WinSize, eTypeWinResize::Show);
	//			IsMinimized = false;
	//		}
	//		else
	//		{
	//			OnResize(m_WinSize, eTypeWinResize::Resize);
	//		}
	//	}

		return 0;
	}

	// Обрабатываем изменение размера окна в процессе изменения его пользователем.
	//case WM_SIZING:
	//	OnResizing();
	//	return 0;

		// Перехватываем это сообщение, чтобы не допустить слишком маленького/большого размера окна.
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);

		DWORD dwStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_STYLE));
		DWORD dwExStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_EXSTYLE));
		BOOL bMenu = (GetMenu(hWnd) != NULL);

		// Минимальный размер клиентской области
		RECT minRect = { 0, 0, static_cast<LONG>(c_MinWinSize), static_cast<LONG>(c_MinWinSize) };
		AdjustWindowRectEx(&minRect, dwStyle, bMenu, dwExStyle);
		pMinMaxInfo->ptMinTrackSize.x = minRect.right - minRect.left;
		pMinMaxInfo->ptMinTrackSize.y = minRect.bottom - minRect.top;

		// Максимальный размер клиентской области
		RECT maxRect = { 0, 0, static_cast<LONG>(c_UHD_4K.GetWidth()), static_cast<LONG>(c_UHD_4K.GetHeight()) };
		AdjustWindowRect(&maxRect, WS_OVERLAPPEDWINDOW, FALSE);
		pMinMaxInfo->ptMaxTrackSize.x = maxRect.right - maxRect.left;
		pMinMaxInfo->ptMaxTrackSize.y = maxRect.bottom - maxRect.top;

		return 0;
	}

	// Обрабатываем изменение DPI в системе
	//case WM_DPICHANGED:
	//{
	//	// Новый DPI
	//	UINT dpiX = LOWORD(wParam);
	//	UINT dpiY = HIWORD(wParam);
	//	RECT* const prcNewWindow = reinterpret_cast<RECT*>(lParam);
	//	SetWindowPos(
	//		hWnd,
	//		nullptr,
	//		prcNewWindow->left,
	//		prcNewWindow->top,
	//		prcNewWindow->right - prcNewWindow->left,
	//		prcNewWindow->bottom - prcNewWindow->top,
	//		SWP_NOZORDER | SWP_NOACTIVATE);

	//	RECT clientRect;
	//	GetClientRect(hWnd, &clientRect);

	//	m_WinSize.width = clientRect.right - clientRect.left;
	//	m_WinSize.height = clientRect.bottom - clientRect.top;

	//	OnResize(m_WinSize, eTypeWinResize::Resize);
	//	return 0;
	//}

	//case WM_MOUSEMOVE:
	//	if (!mouseInside)
	//	{
	//		mouseInside = true;
	//		OnMouseEnter(true);

	//		TRACKMOUSEEVENT tme = {};
	//		tme.cbSize = sizeof(tme);
	//		tme.dwFlags = TME_LEAVE;
	//		tme.hwndTrack = hWnd;
	//		TrackMouseEvent(&tme);
	//	}

	//	break;

	//case WM_MOUSELEAVE:
	//	mouseInside = false;
	//	OnMouseEnter(false);
	//	return 0;

	//case WM_SETFOCUS:
	//	OnFocus(true);
	//	break;

	//case WM_KILLFOCUS:
	//	OnFocus(false);
	//	break;

	//case WM_ACTIVATE:
	//	b_IsWinActive = (wParam != 0);
	//	OnActivate(b_IsWinActive);
	//	return 0;

	//case WM_INPUT:
	//	OnRawInput(reinterpret_cast<HRAWINPUT>(lParam));
	//	return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif // defined(Z_WINDOWS)