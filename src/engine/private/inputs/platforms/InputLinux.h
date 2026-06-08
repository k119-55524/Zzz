#pragma once

#if defined(Z_LINUX)

#include "../IInput.h"

namespace zzz::engine
{
	class InputLinux final : public IInput
	{
	public:
		InputLinux() = default;
		~InputLinux() override = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() override;
		bool ProcessMessage(void* nativeMsg) override;
	};
}
#endif // defined(Z_LINUX)
