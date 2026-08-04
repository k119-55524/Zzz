#pragma once


#include "Window_Common.h"
#include "../../header.h"

namespace zzz::engine
{
	class WinMacOS final : public WindowBase
	{
	public:
		WinMacOS() = delete;
		WinMacOS(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinMacOS() override;

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::string_view appName);
	};
}
