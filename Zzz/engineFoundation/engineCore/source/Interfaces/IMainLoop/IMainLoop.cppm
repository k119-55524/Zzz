#include "pch.h"
export module IMainLoop;

import Event;

export namespace zzz::core
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