#include "pch.h"
export module IMainLoop;

namespace zzz::platforms
{
	export class IMainLoop
	{
	public:
		IMainLoop() = delete;
		IMainLoop(const IMainLoop&) = delete;
		IMainLoop(IMainLoop&&) = delete;

		IMainLoop(std::function<void()> _updateSystem);

		virtual ~IMainLoop() = default;

		IMainLoop& operator=(const IMainLoop&) = delete;
		IMainLoop& operator=(IMainLoop&&) = delete;

		virtual void Run() = 0;

	protected:
		std::function<void()> updateSystem;
	};

	IMainLoop::IMainLoop(std::function<void()> _updateSystem) :
		updateSystem{ std::move(_updateSystem) }
	{
		ensure(updateSystem);
	}
}