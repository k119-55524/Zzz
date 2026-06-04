#pragma once

#include "../core/config/ConfigManager.h"
#include "../platforms/native_view/window/MSWindows.h"
#include "../platforms/native_view/window/LinuxWayland.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	typedef zzz::engine::MSWindows Window;
#elif defined(Z_LINUX)
	typedef zzz::engine::LinuxWayland Window;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class PlatformFactory final
	{
	public:
		std::shared_ptr<IWindow> CreateAppWin(const EngineConfig& config);
	};
}