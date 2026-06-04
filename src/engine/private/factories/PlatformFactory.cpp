#include "PlatformFactory.h"

using namespace zzz;
using namespace zzz::engine;

std::shared_ptr<IWindow> PlatformFactory::CreateAppWin(const PlatformConfig& platformConfig)
{
	try
	{
		return safe_make_shared<Window>(platformConfig);
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