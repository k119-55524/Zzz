#pragma once

#if defined(Z_ANDROID)

#include "../IInput.h"
#include <android/input.h>

namespace zzz::engine
{
	class InputAndroid final : public IInput
	{
	public:
		InputAndroid() = default;
		~InputAndroid() override = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() override;

		// Для Android системные сообщения пробрасываются через AInputEvent
		bool ProcessMessage(void* nativeMsg) override;

		// Специфичный для Android метод
		int32_t HandleInput(AInputEvent* event);
	};
}
#endif // defined(Z_ANDROID)
