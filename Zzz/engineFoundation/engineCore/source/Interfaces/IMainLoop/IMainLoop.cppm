#include "pch.h"
export module IMainLoop;

import event;

export namespace zzz::engineCore
{
	class IMainLoop
	{
		Z_NO_COPY_MOVE(IMainLoop);

	public:
		IMainLoop() = default;
		virtual ~IMainLoop() = default;

		virtual void Run() = 0;

		event<void> onUpdateSystem;
	};
}