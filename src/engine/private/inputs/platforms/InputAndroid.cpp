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



#endif // defined(Z_ANDROID)
