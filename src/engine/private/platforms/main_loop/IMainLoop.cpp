#include "IMainLoop.h"

using namespace zzz::engine;

IMainLoop::IMainLoop(const std::shared_ptr<IPlatform> platform) :
	m_Platform{platform},
	isRunning{true}
{
	ensure(m_Platform != nullptr, "Platform must not be null.");
}