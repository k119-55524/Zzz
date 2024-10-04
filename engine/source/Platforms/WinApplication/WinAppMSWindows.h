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

		zResult Initialize(const DataEngineInitialization& data) override;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HWND hWnd;
		const string wstring_to_string(const wstring& wstr) const;

		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}
#endif // _WINDOWS