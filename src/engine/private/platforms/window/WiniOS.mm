
#include "WiniOS.h"

using namespace zzz::engine;

WiniOS::WiniOS(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input) :
	WindowBase(platform, input)
{
}

WiniOS::~WiniOS()
{
}

std::expected<void, std::string> WiniOS::Initialize(const std::string_view appName)
{
	return {};
}

