#if defined(Z_WINDOWS)

#include "MSWin_Window.h"

using namespace zzz::engine;

MSWin_Window::MSWin_Window(const PlatformConfig& platformConfig) :
	IWindow(platformConfig)
{
}

MSWin_Window::~MSWin_Window()
{
}

[[nodiscard]] std::expected<void, std::string> MSWin_Window::Initialize()
{
	HICON iconHandle = (HICON)LoadImage(GetModuleHandle(NULL), "IDI_ICON1", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MSWin_Window::WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = iconHandle;
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = m_PlatformConfig.GetClassName().c_str();
	ATOM Result = RegisterClass(&wc);
	if (Result == 0)
		UNEXPECTED("Failed to register window class. Error code: {}.", GetLastError());

	return std::expected<void, std::string>();
}

LRESULT CALLBACK MSWin_Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#endif // defined(Z_WINDOWS)