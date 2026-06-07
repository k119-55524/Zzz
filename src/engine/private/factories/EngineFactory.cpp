#include <foundation.h>

#include "EngineFactory.h"

using namespace zzz;
using namespace zzz::engine;

std::shared_ptr<IWindow> EngineFactory::CreateAppWin(const std::shared_ptr<IPlatform> platform)
{
	try
	{
		return safe_make_shared<Window>(platform);
	}
	catch (const std::exception& e)
	{
		THROW_RUNTIME("Failed to create application window: {}.", std::string(e.what()));
	}
	catch (...)
	{
		THROW_RUNTIME("Unknown exception occurred while creating application window.");
	}
}

std::shared_ptr<IMainLoop> EngineFactory::CreateMainLoop(std::shared_ptr<IPlatform> platform)
{
	try
	{
		return safe_make_shared<MainLoop>(platform);
	}
	catch (const std::exception& e)
	{
		THROW_RUNTIME("Failed to create main loop: {}.", e.what());
	}
	catch (...)
	{
		THROW_RUNTIME("Unknown exception occurred while creating main loop.");
	}
}
