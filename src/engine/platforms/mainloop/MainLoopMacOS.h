#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MACOS)

#include "MainLoopCommon.h"

namespace zzz::engine
{
	class MainLoopMacOS final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoopMacOS);

	public:
		MainLoopMacOS() = delete;
		MainLoopMacOS(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoopMacOS() = default;

		void Run() override;
	};
}

#endif // defined(Z_MACOS)
