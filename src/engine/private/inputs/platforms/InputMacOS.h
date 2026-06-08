#pragma once

#if defined(Z_MACOS)

#include "../IInput.h"

namespace zzz::engine
{
	class InputMacOS final : public IInput
	{
	public:
		InputMacOS() = default;
		~InputMacOS() override = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() override;
		bool ProcessMessage(void* nativeMsg) override;
	};
}
#endif // defined(Z_MACOS)
