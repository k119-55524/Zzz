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
		MainLoopBase(const std::shared_ptr<Platform> platform, std::function<void()> onUpdate);
		virtual ~MainLoopBase() = default;

		virtual void Run() = 0;
		inline void Stop() {  isRunning.store(false); }

		protected:
			std::function<void()> OnUpdate;
			std::atomic<bool> isRunning;
			const std::shared_ptr<Platform> m_Platform;
	};
}