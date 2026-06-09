#pragma once

#if defined(Z_ANDROID)

#include "../IInput.h"

namespace zzz::engine
{
	class InputAndroid final : public IInput
	{
	public:
		InputAndroid() = default;
		~InputAndroid() override = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() override;

		bool ProcessMessage(const NativeMsg& nativeMsg) override;

		// Специфичный для Android метод
		int32_t HandleInput(AInputEvent* event);
	};
}
#endif // defined(Z_ANDROID)
