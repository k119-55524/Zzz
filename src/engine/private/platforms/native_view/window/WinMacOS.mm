#if defined(Z_MACOS)

#include "WinMacOS.h"

using namespace zzz::engine;

WinMacOS::WinMacOS(const std::shared_ptr<Platform> platform, const std::shared_ptr<IInput> input) :
	IWindow(platform, input)
{
}

WinMacOS::~WinMacOS()
{
}

std::expected<void, std::string> WinMacOS::Initialize(const std::string_view appName)
{
	return {};
}

#endif // defined(Z_MACOS)
