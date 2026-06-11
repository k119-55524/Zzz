#include "MainLoop_Common.h"

using namespace zzz::engine;

MainLoopBase::MainLoopBase(const std::shared_ptr<Platform> platform, std::function<void()> onUpdate) :
	m_Platform{platform},
	OnUpdate{onUpdate},
	isRunning{true}
{
	ensure(m_Platform != nullptr, "Platform must not be null.");
	ensure(OnUpdate != nullptr, "OnUpdate must not be null.");
}