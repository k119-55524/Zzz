#pragma once

#include <atomic>
#include <core/utils/Types.h>
#include <core/utils/Ensure.h>
#include <core/utils/Defines.h>
#include <core/utils/Macroses.h>
#include <core/utils/MemoryUtils.h>
#include <core/utils/ThrowWrappers.h>

#include "../Platform.h"
#include <core/events/Event.h>

namespace zzz::engine
{
	class MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoopBase);

	public:
		MainLoopBase() = delete;
		MainLoopBase(const Platform& platform, std::function<void()> onUpdate);
		virtual ~MainLoopBase() = default;

		virtual void Run() = 0;
		inline void Stop() {  isRunning.store(false); }

		protected:
			std::function<void()> OnUpdate;
			std::atomic<bool> isRunning;
			const Platform& m_Platform;
	};
}