#pragma once

#if defined(Z_IOS)

#include "../IInput.h"

namespace zzz::engine
{
	class InputiOS final : public IInput
	{
	public:
		InputiOS() = default;
		~InputiOS() override = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() override;
		bool ProcessMessage(const NativeMsg& nativeMsg) override;
	};
}
#endif // defined(Z_IOS)
