#if defined(Z_MACOS)

#include "InputMacOS.h"

using namespace zzz::engine;

std::expected<void, std::string> InputMacOS::Initialize()
{
	return {};
}

bool InputMacOS::ProcessMessage(void* nativeMsg)
{
	return false;
}

#endif // defined(Z_MACOS)
