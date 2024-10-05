#pragma once

#include "IWinApp.h"

#ifdef _WINDOWS
namespace Zzz::Platforms
{
	class WinAppMSWindows : public IWinApp
	{
	public:
		WinAppMSWindows() = delete;
		WinAppMSWindows(WinAppMSWindows&) = delete;
		WinAppMSWindows(WinAppMSWindows&&) = delete;

		WinAppMSWindows(function<void(const zSize& size, e_TypeWinAppResize resType)> _resizeWindows);
		virtual ~WinAppMSWindows();

		zResult Initialize(const DataEngineInitialization& data) override;

	private:
		HWND hWnd;
		bool IsMinimized;

		const string wstring_to_string(const wstring& wstr) const;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
	};
}
#endif // _WINDOWS