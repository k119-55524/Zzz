#if defined(Z_IOS)

#include "InputiOS.h"

using namespace zzz::engine;

std::expected<void, std::string> InputiOS::Initialize()
{
	return {};
}

bool InputiOS::ProcessMessage(void* nativeMsg)
{
	return false;
}

#endif // defined(Z_IOS)
