#if defined(Z_ANDROID)

#include "InputAndroid.h"

using namespace zzz::engine;

std::expected<void, std::string> InputAndroid::Initialize()
{
	return {};
}

bool InputAndroid::ProcessMessage(const NativeMsg& nativeMsg)
{
	// На Android это будет вызываться реже, так как основной вход через HandleInput
	return false;
}

int32_t InputAndroid::HandleInput(AInputEvent* event)
{
	// Минимальная логика обработки (пока просто возвращаем 0 - не обработано)
	return 0;
}

#endif // defined(Z_ANDROID)
