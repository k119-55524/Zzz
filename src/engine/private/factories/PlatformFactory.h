#pragma once

#include "../core/config/ConfigManager.h"
#include "../platforms/native_view/window/MSWin_Window.h"
#include "../platforms/native_view/window/Linux_Window.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	typedef zzz::engine::MSWin_Window Window;
#elif defined(Z_LINUX)
	typedef zzz::engine::Linux_Window Window;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class PlatformFactory final
	{
	public:
		std::shared_ptr<IWindow> CreateAppWin(const EngineConfig& config);
	};
}