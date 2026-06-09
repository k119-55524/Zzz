#pragma once

#if defined(Z_IOS)

#include "IWindow.h"
#include "../../../../header.h"

namespace zzz::engine
{
	class WiniOS final : public IWindow
	{
	public:
		WiniOS() = delete;
		WiniOS(const std::shared_ptr<Platform> platform, const std::shared_ptr<IInput> input);
		~WiniOS() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;
	};
}
#endif // defined(Z_IOS)
