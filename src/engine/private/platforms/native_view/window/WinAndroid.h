#pragma once

#if defined(Z_ANDROID)

#include "IWindow.h"

#include "../../../../header.h"

namespace zzz::engine
{
	class WinAndroid final : public IWindow
	{
	public:
		WinAndroid() = delete;
		WinAndroid(const std::shared_ptr<IPlatform> platform, const std::shared_ptr<IInput> input);
		~WinAndroid() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;
	};
}
#endif // defined(Z_ANDROID)
