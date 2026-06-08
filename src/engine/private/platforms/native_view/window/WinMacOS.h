#pragma once

#if defined(Z_MACOS)

#include "IWindow.h"
#include "../../../../header.h"

namespace zzz::engine
{
	class WinMacOS final : public IWindow
	{
	public:
		WinMacOS() = delete;
		WinMacOS(const std::shared_ptr<IPlatform> platform, const std::shared_ptr<IInput> input);
		~WinMacOS() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;
	};
}
#endif // defined(Z_MACOS)
