#include "PlatformFactory.h"

using namespace zzz;
using namespace zzz::engine;

std::shared_ptr<IWindow> PlatformFactory::CreateAppWin(std::shared_ptr<ConfigManager> configManager)
{
	try
	{
		return safe_make_shared<Window>(configManager);
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