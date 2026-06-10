#include "MainLoop_Common.h"

using namespace zzz::engine;

MainLoopBase::MainLoopBase(const std::shared_ptr<Platform> platform) :
	m_Platform{platform},
	isRunning{true}
{
	ensure(m_Platform != nullptr, "Platform must not be null.");
}