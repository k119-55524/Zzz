#include "WinMSWindows.h"
#include "../Platform.h"
#include "../../core/utils/ScreenResolution.h"
#include "../../core/config/platforms/ConfigMSWin.h"

using namespace zzz::engine;

WinMSWindows::WinMSWindows(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input) :
	WindowBase(platform, input),
	m_hWnd(nullptr)
{
}

WinMSWindows::~WinMSWindows()
{
}

[[nodiscard]] std::expected<void, std::string> WinMSWindows::Initialize(const std::string_view appName)
{
	// Рассчитать размеры прямоугольника окна на основе запрошенных размеров клиентской области.
	Size2D<LONG> winSize;
	// TODO: не правильная архитектура. Подумать как задавать размер окна
	winSize.SetFrom(c_DefaultWindowWidth, c_DefaultWindowHeicht);
	RECT R = { 0, 0, winSize.width, winSize.height };
	AdjustWindowRectEx(&R, WS_OVERLAPPEDWINDOW, false, 0);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
	int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
	int xPos = (screenWidth - width) / 2;  // Расчет позиции по оси X
	int yPos = (screenHeight - height) / 2; // Расчет позиции по оси Y

	m_Ctx = { this, m_Input.get() };

	CreateWindowEx(
		0,
		static_cast<const ConfigMSWin&>(m_Platform->GetPlatformConfig()).GetClassName().c_str(),
		appName.data(),
		WS_OVERLAPPEDWINDOW,
		xPos, yPos, width, height,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		&m_Ctx);

	if (!m_hWnd)
		THROW_RUNTIME("CreateWindowEx( ... ) failed. Error code (Windows): {}", ::GetLastError());

	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);

	return std::expected<void, std::string>();
}

WinMSWindows::MsgProcResult WinMSWindows::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCCREATE:
		m_hWnd = hWnd;

		return { false, TRUE };

	case WM_CLOSE:
		onCloseRequested();

		return { false, DefWindowProc(hWnd, uMsg, wParam, lParam) };

	case WM_SIZE:
		m_WinSize.SetFrom(static_cast<zU32>(LOWORD(lParam)), static_cast<zU32>(HIWORD(lParam)));
		DOut("WM_SIZE {}: {}.", static_cast<void*>(m_hWnd), m_WinSize.ToString());

		return { false, 0 };

	case WM_GETMINMAXINFO:
		MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
		DWORD dwStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_STYLE));
		DWORD dwExStyle = static_cast<DWORD>(GetWindowLongPtr(hWnd, GWL_EXSTYLE));
		BOOL bMenu = (GetMenu(hWnd) != NULL);

		RECT minRect = { 0, 0, static_cast<LONG>(c_MinWinSize), static_cast<LONG>(c_MinWinSize) };
		AdjustWindowRectEx(&minRect, dwStyle, bMenu, dwExStyle);
		pMinMaxInfo->ptMinTrackSize.x = minRect.right - minRect.left;
		pMinMaxInfo->ptMinTrackSize.y = minRect.bottom - minRect.top;

		RECT maxRect = { 0, 0, static_cast<LONG>(c_UHD_4K.GetWidth()), static_cast<LONG>(c_UHD_4K.GetHeight()) };
		AdjustWindowRect(&maxRect, WS_OVERLAPPEDWINDOW, FALSE);
		pMinMaxInfo->ptMaxTrackSize.x = maxRect.right - maxRect.left;
		pMinMaxInfo->ptMaxTrackSize.y = maxRect.bottom - maxRect.top;

		return { false, 0 };
	}

	return { true, 0 };
}