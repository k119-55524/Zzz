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
		~WinAppMSWindows() override;

		void Initialize(const DataEngineInitialization& data) override;
		inline const HWND GetHWND() const noexcept { return hWnd; };

	private:
		HWND hWnd;
		bool IsMinimized;

		static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
		const string WstringToString(const zStr& wstr);
	};
}
#endif // _WINDOWS