#include "pch.h"
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

zResult WinAppMSWindows::Initialize(const DataEngineInitialization& data)
{
	auto className = data.GetWinData()->GetWinClassName();

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WinAppMSWindows::WindowProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(data.GetWinData()->GetIcoID()));
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = className.c_str();

	ATOM result = RegisterClass(&wc);
	if (result == 0)
	{
		string mess = ">>>>> [WinAppMSWindows::Initialize( ... )].\n+--- Failed to register window class: ";
		mess += wstring_to_string(data.GetWinData()->GetWinClassName());
		mess += "\n+--- error code(Windows): ";
		mess += to_string(::GetLastError());
		throw runtime_error(mess);
	}

	// ���������� ������� �������������� ���� �� ������ ����������� �������� ���������� �������.
	RECT R = { 0, 0, static_cast<LONG>(data.GetWinSize().width), static_cast<LONG>(data.GetWinSize().height) };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);  // ������ ������
	int screenHeight = GetSystemMetrics(SM_CYSCREEN); // ������ ������
	int xPos = (screenWidth - width) / 2;  // ������ ������� �� ��� X
	int yPos = (screenHeight - height) / 2; // ������ ������� �� ��� Y
	hWnd = CreateWindowEx(
		0,
		className.c_str(),
		data.GetWinData()->GetWinCaption().c_str(),
		WS_OVERLAPPEDWINDOW,
		xPos, yPos, width, height,
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		this
	);

	if (!hWnd)
	{
		string mess = ">>>>> [WinAppMSWindows::Initialize( ... )].\n+--- Failed to create window: ";
		mess += "+--- error code(Windows): ";
		mess += to_string(::GetLastError());
		throw runtime_error(mess);
	}

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	return zResult();
}

const string WinAppMSWindows::wstring_to_string(const wstring& wstr) const
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);

	return str;
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

		// ������������� ��� ���������, ����� �� ��������� ������� ����������/�������� ������� ����.
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

		// ����������� ��������� DPI � �������
	case WM_DPICHANGED:
	{
		// �������� ����� �������� DPI
		int newDPI = LOWORD(wParam);

		// ������������� ����� ������ ����, ���� ��� ����������
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
	//	// ����� ����� ��������� ���������, ���� ��� ����������.
	//	EndPaint(hWnd, &ps);
	//	return 0;
	//}

	default:
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

#endif // _WINDOWS