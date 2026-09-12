#pragma once

#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include "MainLoopCommon.h"

namespace zzz::engine
{
	class MainLoopAndroid final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoopAndroid);

	public:
		MainLoopAndroid() = delete;
		MainLoopAndroid(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoopAndroid() = default;

		void Run() override;
	};
}

#endif // defined(Z_ANDROID)
