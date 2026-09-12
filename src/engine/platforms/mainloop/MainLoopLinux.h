#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include "MainLoopCommon.h"

struct wl_display;

namespace zzz::engine
{
	class MainLoopLinux final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoopLinux);

	public:
		MainLoopLinux() = delete;
		MainLoopLinux(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoopLinux() = default;

		void Run() override;

	private:
		wl_display* m_Display;
	};
}

#endif // defined(Z_LINUX)
