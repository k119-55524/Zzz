#if defined(Z_IOS)

#include "WiniOS.h"

using namespace zzz::engine;

WiniOS::WiniOS(const std::shared_ptr<IPlatform> platform, const std::shared_ptr<IInput> input) :
	IWindow(platform, input)
{
}

WiniOS::~WiniOS()
{
}

std::expected<void, std::string> WiniOS::Initialize(const std::string_view appName)
{
	return {};
}

#endif // defined(Z_IOS)
