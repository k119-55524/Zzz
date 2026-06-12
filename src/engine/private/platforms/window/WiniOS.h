#pragma once


#include "Window_Common.h"
#include "../../../header.h"

namespace zzz::engine
{
	class WiniOS final : public WindowBase
	{
	public:
		WiniOS() = delete;
		WiniOS(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WiniOS() override;

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::string_view appName);
	};
}
