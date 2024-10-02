#pragma once

#include "IWinApp.h"

#ifdef _WINDOWS
namespace Zzz::Platforms
{
	class WinAppMSWindows : public IWinApp
	{
	public:
		WinAppMSWindows();
		WinAppMSWindows(WinAppMSWindows&) = delete;
		WinAppMSWindows(WinAppMSWindows&&) = delete;

		virtual ~WinAppMSWindows();

		zResult Initialize(const shared_ptr<IInitWinData> data) override;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HWND hwnd;
		string wstring_to_string(const wstring& wstr);

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}
#endif // _WINDOWS