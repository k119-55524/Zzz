#include "pch.h"
#include "WinAppMSWindows.h"

using namespace Zzz;
using namespace Zzz::Platforms;

#ifdef _WINDOWS

WinAppMSWindows::WinAppMSWindows()
{
}

WinAppMSWindows::~WinAppMSWindows()
{
}

zResult WinAppMSWindows::Initialize(const shared_ptr<IInitWinData> data)
{
	zResult res;

	auto msData = dynamic_pointer_cast<InitMSWindowsData>(data);

	if (!msData)
		throw invalid_argument(">>>>> [WinAppMSWindows::Initialize]. Каст dynamic_pointer_cast<InitMSWindowsData>(data) не удался.");

	const wchar_t* CLASS_NAME = msData->GetClassName();// L"zEngineWinClass";

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WinAppMSWindows::WindowProc;	
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = CLASS_NAME;

	ATOM result = RegisterClass(&wc);
	if (result == 0)
	{
		string mess = ">>>>> [WinAppMSWindows::Initialize(const shared_ptr<IInitWinData> data)].\n+--- Не удалось зарегистрировать класс окна: ";
		mess += wstring_to_string(msData->GetClassName());
		mess += "    Код ошибки: ";
		mess += to_string(::GetLastError());
		throw runtime_error(mess);
	}

	return res;
}

string WinAppMSWindows::wstring_to_string(const wstring& wstr)
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

		pThis->hwnd = hwnd;
	}
	else
		pThis = (WinAppMSWindows*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

	if (pThis)
		return pThis->HandleMessage(uMsg, wParam, lParam);

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

#endif // _WINDOWS