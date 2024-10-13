#include "pch.h"
#include "../Platform.h"
#include "WinAppMSWindows.h"

using namespace Zzz;
using namespace Zzz::Platforms;

#ifdef _WINDOWS

WinAppMSWindows::WinAppMSWindows(function<void(const zSize& size, e_TypeWinAppResize resType)> _resizeWindows) :
	IWinApp(_resizeWindows),
	hWnd{ nullptr },
	IsMinimized{true}
{
	SetProcessDPIAware();
}

WinAppMSWindows::~WinAppMSWindows()
{
	if (hWnd)
		DestroyWindow(hWnd);
}

void WinAppMSWindows::Initialize(const shared_ptr<UserGameSettings> userGS)
{
	auto className = userGS->GetMSWinClassName();

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinAppMSWindows::WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(userGS->GetMSWinIcoID()));
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = className.c_str();

	ATOM result = RegisterClass(&wc);
	if (result == 0)
	{
		string mess = "RegisterClass( ... ). Failed to register window class: ";
		mess += WstringToString(className);
		mess += "\n+--- error code(Windows): ";
		mess += to_string(::GetLastError());
		THROW_RUNTIME_ERROR(mess);
	}

	// Рассчитать размеры прямоугольника окна на основе запрошенных размеров клиентской области.
	RECT R = { 0, 0, static_cast<LONG>(userGS->GetSetupAppWinSize().width), static_cast<LONG>(userGS->GetSetupAppWinSize().height)};
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // Ширина экрана
	int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Высота экрана
	int xPos = (screenWidth - width) / 2;  // Расчет позиции по оси X
	int yPos = (screenHeight - height) / 2; // Расчет позиции по оси Y
	hWnd = CreateWindowEx(
		0,
		className.c_str(),
		userGS->GetWinCaption().c_str(),
		WS_OVERLAPPEDWINDOW,
		xPos, yPos, width, height,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		this);

	if (!hWnd)
	{
		string mess = "CreateWindowEx( ... ). Failed to create window: ";
		mess += "\n +--- error code(Windows): ";
		mess += to_string(::GetLastError());
		THROW_RUNTIME_ERROR(mess);
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}

LRESULT CALLBACK WinAppMSWindows::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WinAppMSWindows* pThis;

	if (uMsg == WM_NCCREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		pThis = (WinAppMSWindows*)pCreate->lpCreateParams;
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

		pThis->hWnd = hwnd;
	}
	else
		pThis = (WinAppMSWindows*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pThis)
		return pThis->MsgProc(uMsg, wParam, lParam);

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT WinAppMSWindows::MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		winSize.width = static_cast<zU64>(LOWORD(lParam));
		winSize.height = static_cast<zU64>(HIWORD(lParam));
		if (resizeWindows != nullptr)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				resizeWindows(winSize, e_TypeWinAppResize::eHide);
				IsMinimized = true;
			}
			else
			{
				if ((wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) && IsMinimized)
				{
					resizeWindows(winSize, e_TypeWinAppResize::eShow);
					IsMinimized = false;
				}
				else
				{
					resizeWindows(winSize, e_TypeWinAppResize::eResize);
				}
			}
		}
		return 0;

		// Перехватываем это сообщение, чтобы не допустить слишком маленького/большого размера окна.
	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = c_MinimumWindowsWidth;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = c_MinimumWindowsHeight;

		RECT R = { 0, 0, static_cast<LONG>(c_MaximumWindowsWidth), static_cast<LONG>(c_MaximumWindowsHeight) };
		AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
		LONG maxWidth = R.right - R.left;
		LONG maxHeight = R.bottom - R.top;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = maxWidth;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = maxHeight;
		return 0;
	}

		// брабатываем изменение DPI в системе
	case WM_DPICHANGED:
	{
		// Получаем новое значение DPI
		int newDPI = LOWORD(wParam);

		// Устанавливаем новый размер окна, если это необходимо
		RECT* prc = (RECT*)lParam;
		SetWindowPos(hWnd, NULL, prc->left, prc->top,
			prc->right - prc->left, prc->bottom - prc->top,
			SWP_NOZORDER | SWP_NOACTIVATE);
		return 0;
	}

	//case WM_PAINT:
	//{
	//	PAINTSTRUCT ps;
	//	HDC hdc = BeginPaint(hWnd, &ps);
	//	// Здесь можно выполнить рисование, если это необходимо.
	//	EndPaint(hWnd, &ps);
	//	return 0;
	//}

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

const string WinAppMSWindows::WstringToString(const zStr& wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);

	return str;
}

#endif // _WINDOWS