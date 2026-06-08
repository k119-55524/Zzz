#pragma once

#if defined(Z_WINDOWS)

#include "../IInput.h"

namespace zzz::engine
{
	class InputMSWindows final : public IInput
	{
	public:
		InputMSWindows() = default;
		~InputMSWindows() override = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() override;
		bool ProcessMessage(void* nativeMsg) override;
	};
}
#endif // defined(Z_WINDOWS)
