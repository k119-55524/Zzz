#pragma once

#include <atomic>
#include <foundation.h>

#include "../platforms/IPlatform.h"
#include "../../core/templates/Event.h"

namespace zzz::engine
{
	class IMainLoop
	{
		Z_NO_COPY_MOVE(IMainLoop);

	public:
		IMainLoop() = delete;
		IMainLoop(const std::shared_ptr<IPlatform> platform);
		virtual ~IMainLoop() = default;

		virtual void Run() = 0;
		inline void Stop() noexcept { isRunning.store(false); }

		Event<void> onUpdateSystem;

		protected:
			std::atomic<bool> isRunning;
			const std::shared_ptr<IPlatform> m_Platform;
	};
}