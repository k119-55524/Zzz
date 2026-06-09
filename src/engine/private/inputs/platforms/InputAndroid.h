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

	};
}
#endif // defined(Z_ANDROID)
