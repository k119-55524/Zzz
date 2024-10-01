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

	protected:
		zResult Initialize(const s_zEngineInit::WinAppSettings* data) override;
	};
}
#endif // _WINDOWS