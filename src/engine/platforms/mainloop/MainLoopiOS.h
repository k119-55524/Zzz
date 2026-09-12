#pragma once

#include "core/utils/Defines.h"

#if defined(Z_IOS)

#include "MainLoopCommon.h"

namespace zzz::engine
{
	class MainLoopiOS final : public MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoopiOS);

	public:
		MainLoopiOS() = delete;
		MainLoopiOS(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoopiOS() = default;

		void Run() override;
	};
}

#endif // defined(Z_IOS)
