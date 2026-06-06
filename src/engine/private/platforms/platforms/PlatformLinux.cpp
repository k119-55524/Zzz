#if defined(Z_LINUX)

#include "PlatformLinux.h"

using namespace zzz::engine;

PlatformLinux::PlatformLinux(std::string_view appName, std::shared_ptr<void> platformData) :
	IPlatform(appName, platformData)
{
}

PlatformLinux::~PlatformLinux()
{

}

void PlatformLinux::InitializeImpl()
{

}
#endif // defined(Z_LINUX)