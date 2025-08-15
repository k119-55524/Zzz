#include "pch.h"
export module IMainLoop;

import event;

using namespace zzz;

export namespace zzz::platforms
{
	class IMainLoop
	{
	public:
		IMainLoop() = default;
		IMainLoop(const IMainLoop&) = delete;
		IMainLoop(IMainLoop&&) = delete;

		IMainLoop& operator=(const IMainLoop&) = delete;
		IMainLoop& operator=(IMainLoop&&) = delete;

		virtual ~IMainLoop() = default;

		virtual void Run() = 0;

		event<void> onUpdateSystem;
	};
}