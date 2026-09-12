#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include "MainLoopCommon.h"

namespace zzz::engine
{
	class MainLoopMSWin final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoopMSWin);

	public:
		MainLoopMSWin() = delete;
		MainLoopMSWin(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoopMSWin() = default;

		void Run() override;
	};
}

#endif // defined(Z_WINDOWS)
