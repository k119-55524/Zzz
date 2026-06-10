#pragma once


#include "Window.h"
#include "../../../header.h"

namespace zzz::engine
{
	class WiniOS final : public Window
	{
	public:
		WiniOS() = delete;
		WiniOS(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input);
		~WiniOS() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;
	};
}
