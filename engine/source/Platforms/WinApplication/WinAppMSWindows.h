#pragma once

#include "WinAppBase.h"

#if defined(_WINDOWS)
namespace Zzz::Platforms
{
	class WinAppMSWindows : public WinAppBase
	{
	public:
		WinAppMSWindows();
		WinAppMSWindows(WinAppMSWindows&) = delete;
		WinAppMSWindows(WinAppMSWindows&&) = delete;

	protected:
		zResult Init(const s_zEngineInit::WinAppSettings* data) override;
	};
}
#endif // defined(_WINDOWS)