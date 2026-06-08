#include <foundation.h>

#include "PlatformFactory.h"

using namespace zzz;
using namespace zzz::engine;

std::shared_ptr<IPlatform> PlatformFactory::Create(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData)
{
	try
	{
		auto platform = safe_make_shared<Platform>(appName, platformData);
		platform->Initialize();
		return platform;
	}
	catch (const std::exception& e)
	{
		THROW_RUNTIME("Failed to create platform: {}.", e.what());
	}
	catch (...)
	{
		THROW_RUNTIME("Unknown exception occurred while creating platform.");
	}
}
