#include "MainLoop_Common.h"

using namespace zzz::engine;

MainLoopBase::MainLoopBase(const Platform& platform, std::function<void()> onUpdate) :
	m_Platform{platform},
	OnUpdate{onUpdate},
	isRunning{true}
{
	ensure(OnUpdate != nullptr, "OnUpdate не должен быть null.");
}