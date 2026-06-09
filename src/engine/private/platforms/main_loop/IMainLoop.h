#pragma once

#include <atomic>
#include <foundation.h>

#include "../Platform.h"
#include "../../core/templates/Event.h"

namespace zzz::engine
{
	class IMainLoop
	{
		Z_NO_COPY_MOVE(IMainLoop);

	public:
		IMainLoop() = delete;
		IMainLoop(const std::shared_ptr<Platform> platform);
		virtual ~IMainLoop() = default;

		virtual void Run() = 0;
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