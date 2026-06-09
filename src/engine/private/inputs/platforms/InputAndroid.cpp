#if defined(Z_ANDROID)

#include "InputAndroid.h"

using namespace zzz::engine;

std::expected<void, std::string> InputAndroid::Initialize()
{
	return {};
}

bool InputAndroid::ProcessMessage(const NativeMsg& nativeMsg)
{

	return false;
}

int32_t InputAndroid::HandleInput(AInputEvent* event)
{

	return 0;
}

#endif // defined(Z_ANDROID)
