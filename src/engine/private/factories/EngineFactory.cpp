#include <foundation.h>

#include "EngineFactory.h"

using namespace zzz;
using namespace zzz::engine;

std::shared_ptr<IWindow> EngineFactory::CreateAppWin(const std::shared_ptr<Platform> platform, const std::shared_ptr<IInput> input)
{
	return safe_make_shared<Window>(platform, input);
}

std::shared_ptr<IMainLoop> EngineFactory::CreateMainLoop(std::shared_ptr<Platform> platform)
{
	return safe_make_shared<MainLoop>(platform);
}

std::shared_ptr<IInput> EngineFactory::CreateInput()
{
	return safe_make_shared<Input>();
}
