#if defined(Z_ANDROID)

#include "WinAndroid.h"

using namespace zzz::engine;

WinAndroid::WinAndroid(const std::shared_ptr<IPlatform> platform) :
	IWindow(platform)
{
}

WinAndroid::~WinAndroid()
{
}

std::expected<void, std::string> WinAndroid::Initialize(const std::string_view appName)
{
	return {};
}

#endif // defined(Z_ANDROID)
