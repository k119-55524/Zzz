#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include "MainLoopCommon.h"

namespace zzz::engine
{
	class MainLoop_MSWin final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoop_MSWin);

	public:
		MainLoop_MSWin() = delete;
		MainLoop_MSWin(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoop_MSWin() = default;

		void Run() override;
	};
}

#endif // defined(Z_WINDOWS)
