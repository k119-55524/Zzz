#pragma once

#include "../core/config/ConfigManager.h"
#include "../platforms/platforms/IPlatform.h"
#include "../platforms/native_view/window/WinLinux.h"
#include "../platforms/native_view/window/WinMSWindows.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	typedef zzz::engine::MSWindows Window;
#elif defined(Z_LINUX)
	typedef zzz::engine::WinLinux Window;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class PlatformFactory final
	{
	public:
		std::shared_ptr<IWindow> CreateAppWin(const std::shared_ptr<IPlatform> platform);
	};
}