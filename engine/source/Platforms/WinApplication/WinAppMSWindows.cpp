#include "pch.h"
#include "WinAppMSWindows.h"

using namespace Zzz;
using namespace Zzz::Platforms;

#ifdef _WINDOWS

WinAppMSWindows::WinAppMSWindows() :
	hWnd{ nullptr }
{
}

WinAppMSWindows::~WinAppMSWindows()
{
}

zResult WinAppMSWindows::Initialize(const DataEngineInitialization& data)
{
	auto className = data.GetWinData()->GetWinClassName().c_str();

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WinAppMSWindows::WindowProc;	
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = className;

	ATOM result = RegisterClass(&wc);
	if (result == 0)
	{
		string mess = ">>>>> [WinAppMSWindows::Initialize(const shared_ptr<IInitWinData> data)].\n+--- Не удалось зарегистрировать класс окна: ";
		mess += wstring_to_string(data.GetWinData()->GetWinClassName());
		mess += "    Код ошибки(Windows): ";
		mess += to_string(::GetLastError());
		throw runtime_error(mess);
	}

	hWnd = CreateWindowEx(
		0,
		className,
		data.GetWinData()->GetWinCaption().c_str(),
		WS_OVERLAPPEDWINDOW,			// Стиль окна
		CW_USEDEFAULT, CW_USEDEFAULT,	// Позиция окна
		static_cast<int>(data.GetWinSize().width),
		static_cast<int>(data.GetWinSize().height),
		nullptr,
		nullptr,
		GetModuleHandle(NULL),
		nullptr
	);

	return zResult();
}

const string WinAppMSWindows::wstring_to_string(const wstring& wstr) const
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
	string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);

	return str;
}

LRESULT WinAppMSWindows::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return LRESULT();
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
		return pThis->HandleMessage(uMsg, wParam, lParam);

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#endif // _WINDOWS