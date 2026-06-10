#pragma once


#include "Window.h"
#include "../../../header.h"

namespace zzz::engine
{
	class WinMacOS final : public Window
	{
	public:
		WinMacOS() = delete;
		WinMacOS(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input);
		~WinMacOS() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;
	};
}
