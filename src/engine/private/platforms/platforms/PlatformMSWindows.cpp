#if defined(Z_WINDOWS)

#include "PlatformMSWindows.h"

using namespace zzz::engine;

PlatformMSWindows::PlatformMSWindows(std::string_view appName, std::shared_ptr<void> platformData) :
	IPlatform(appName, platformData)
{
}

PlatformMSWindows::~PlatformMSWindows()
{

}

void PlatformMSWindows::InitializeImpl()
{

}
#endif // defined(Z_WINDOWS)