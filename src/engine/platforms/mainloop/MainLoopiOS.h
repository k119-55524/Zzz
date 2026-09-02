#pragma once

#include "core/utils/Defines.h"

#if defined(Z_IOS)

#include "MainLoopCommon.h"

namespace zzz::engine
{
	class MainLoop_iOS final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoop_iOS);

	public:
		MainLoop_iOS() = delete;
		MainLoop_iOS(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoop_iOS() = default;

		void Run() override;
	};
}

#endif // defined(Z_IOS)
