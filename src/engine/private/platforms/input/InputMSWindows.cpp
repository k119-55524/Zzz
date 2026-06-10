
#include "InputMSWindows.h"

using namespace zzz::engine;

std::expected<void, std::string> InputMSWindows::Initialize()
{
	return {};
}

bool InputMSWindows::ProcessMessage(const NativeMsg& nativeMsg)
{
	return true;
}
