#if defined(Z_WINDOWS)

//#include "../../../core/templates/Size2D.h"
#include "MSWin_Window.h"

using namespace zzz::engine;

MSWin_Window::MSWin_Window(const EngineConfig& config) :
	IWindow(config),
	m_hWnd(nullptr)
{
}

MSWin_Window::~MSWin_Window()
{
}

[[nodiscard]] std::expected<void, std::string> MSWin_Window::Initialize(const std::string_view appName)
{
	HICON iconHandle = (HICON)LoadImage(
		GetModuleHandle(NULL),
		m_Config.GetPlatformConfig().GetIcoResourceName().c_str(),
		IMAGE_ICON,
		0,
		0,
		LR_DEFAULTSIZE | LR_SHARED);

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MSWin_Window::WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = m_Config.GetPlatformConfig().GetClassName().c_str();
	ATOM Result = RegisterClass(&wc);
	if (Result == 0)
		return UNEXPECTED("Failed to register window class. Error code: {}.", GetLastError());

	// Рассчитать размеры прямоугольника окна на основе запрошенных размеров клиентской области.
	//const Size2D<LONG>& winSize = m_Config.GetWinSize();
	//RECT R = { 0, 0, winSize.width, winSize.height };
	//AdjustWindowRectEx(&R, WS_OVERLAPPEDWINDOW, false, 0);
	//int width = R.right - R.left;
	//int height = R.bottom - R.top;

	//int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
	//int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
	//int xPos = (screenWidth - width) / 2;  // Расчет позиции по оси X
	//int yPos = (screenHeight - height) / 2; // Расчет позиции по оси Y
	//m_hWnd = CreateWindowEx(
	//	0,
	//	m_Config.GetPlatformConfig().GetClassName().c_str(),
	//	appName.data(),
	//	WS_OVERLAPPEDWINDOW,
	//	xPos, yPos, width, height,
	//	nullptr,
	//	nullptr,
	//	GetModuleHandle(NULL),
	//	this);

	//if (!m_hWnd)
	//	THROW_RUNTIME("CreateWindowEx( ... ) failed. Error code (Windows): {}", ::GetLastError());

	//ShowWindow(m_hWnd, SW_SHOW);
	//UpdateWindow(m_hWnd);

	return std::expected<void, std::string>();
}

LRESULT CALLBACK MSWin_Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#endif // defined(Z_WINDOWS)