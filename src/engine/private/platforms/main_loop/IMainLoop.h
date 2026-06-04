#pragma once

#include <foundation.h>

#include "../../core/templates/Event.h"

namespace zzz::engine
{
	class IMainLoop
	{
		Z_NO_COPY_MOVE(IMainLoop);

	public:
		IMainLoop() = default;
		virtual ~IMainLoop() = default;

		virtual void Run() = 0;

		Event<void> onUpdateSystem;
	};
}