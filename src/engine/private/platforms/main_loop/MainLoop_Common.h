#pragma once

#include <atomic>
#include <foundation.h>

#include "../Platform.h"
#include "../../core/templates/Event.h"

namespace zzz::engine
{
	class MainLoopBase
	{
		Z_NO_COPY_MOVE(MainLoopBase);

	public:
		MainLoopBase() = delete;
		MainLoopBase(const std::shared_ptr<Platform> platform);
		virtual ~MainLoopBase() = default;

		
		inline void Stop()
		{ 
			isRunning.store(false);
		}

		Event<void> onUpdateSystem;

		protected:
			std::atomic<bool> isRunning;
			const std::shared_ptr<Platform> m_Platform;
	};
}