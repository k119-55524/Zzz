#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MACOS)

#include "MainLoopCommon.h"

namespace zzz::engine
{
	class MainLoop_MacOS final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoop_MacOS);

	public:
		MainLoop_MacOS() = delete;
		MainLoop_MacOS(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoop_MacOS() = default;

		void Run() override;
	};
}

#endif // defined(Z_MACOS)
