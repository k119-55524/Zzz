#if defined(Z_LINUX)

#include "InputLinux.h"

using namespace zzz::engine;

std::expected<void, std::string> InputLinux::Initialize()
{
	return {};
}

bool InputLinux::ProcessMessage(const NativeMsg& nativeMsg)
{
	// Разбор событий Wayland/X11
	return false;
}

#endif // defined(Z_LINUX)
