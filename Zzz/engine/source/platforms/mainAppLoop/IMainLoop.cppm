#include "pch.h"
export module IMainLoop;

namespace zzz::platforms
{
	export class IMainLoop
	{
	public:
		IMainLoop() = delete;
		IMainLoop(IMainLoop&) = delete;
		IMainLoop(IMainLoop&&) = delete;

		IMainLoop(std::function<void()> _updateSystem);

		virtual ~IMainLoop() = 0;

		virtual void Run() = 0;

	protected:
		std::function<void()> updateSystem;
	};

	IMainLoop::IMainLoop(std::function<void()> _updateSystem) :
		updateSystem{ _updateSystem }
	{
	}
}