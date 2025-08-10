#include "pch.h"
export module IMainLoop;

import zEvent;

using namespace zzz;

export namespace zzz::platforms
{
	class IMainLoop
	{
	public:
		IMainLoop() = default;
		IMainLoop(const IMainLoop&) = delete;
		IMainLoop(IMainLoop&&) = delete;

		virtual ~IMainLoop() = default;

		IMainLoop& operator=(const IMainLoop&) = delete;
		IMainLoop& operator=(IMainLoop&&) = delete;

		virtual void Run() = 0;

		zEvent<void> onUpdateSystem;

	protected:
		std::function<void()> updateSystem;
	};
}