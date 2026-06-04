#pragma once

#include "PlatformFactory.h"
#include "../core/Config/ConfigManager.h"
#include "../platforms/native_view/window/MSWin_Window.h"

namespace zzz::engine
{
#if defined(Z_WINDOWS)
	typedef zzz::engine::MSWin_Window Window;
#else
#error >>>>> Unsupported platform. No window implementation available.
#endif

	class PlatformFactory final
	{
	public:
		std::shared_ptr<IWindow> CreateAppWin(const EngineConfig& config);
	};
}
