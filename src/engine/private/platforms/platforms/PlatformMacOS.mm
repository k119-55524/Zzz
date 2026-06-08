#if defined(Z_MACOS)

#include "PlatformMacOS.h"

using namespace zzz::engine;

PlatformMacOS::PlatformMacOS(std::string_view appName, std::shared_ptr<PlatformNativeData> platformData) :
	IPlatform(appName, platformData)
{
}

PlatformMacOS::~PlatformMacOS()
{
}

void PlatformMacOS::InitializeImpl()
{
}

#endif // defined(Z_MACOS)
