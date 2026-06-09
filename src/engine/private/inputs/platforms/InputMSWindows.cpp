#if defined(Z_WINDOWS)

#include "InputMSWindows.h"

using namespace zzz::engine;

std::expected<void, std::string> InputMSWindows::Initialize()
{
	return {};
}

bool InputMSWindows::ProcessMessage(const NativeMsg& nativeMsg)
{
	// Здесь будет разбор MSG (uMsg, wParam, lParam)
	return true;
}

#endif // defined(Z_WINDOWS)
