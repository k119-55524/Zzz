#if defined(Z_ANDROID)

#include "PlatformAndroid.h"

using namespace zzz::engine;

PlatformAndroid::PlatformAndroid(std::string_view appName, std::shared_ptr<void> platformData) :
	IPlatform(appName, platformData)
{
}

PlatformAndroid::~PlatformAndroid()
{
}

void PlatformAndroid::InitializeImpl()
{
}

#endif // defined(Z_ANDROID)
