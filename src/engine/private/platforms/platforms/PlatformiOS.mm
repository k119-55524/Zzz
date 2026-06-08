#if defined(Z_IOS)

#include "PlatformiOS.h"

using namespace zzz::engine;

PlatformiOS::PlatformiOS(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData) :
	IPlatform(appName, platformData)
{
}

PlatformiOS::~PlatformiOS()
{
}

void PlatformiOS::InitializeImpl()
{
}

#endif // defined(Z_IOS)
