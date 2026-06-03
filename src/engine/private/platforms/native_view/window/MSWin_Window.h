#pragma once

#if defined(Z_WINDOWS)

#include "IWindow.h"

namespace zzz::engine
{
	class MSWin_Window final : public IWindow
	{
	public:
		MSWin_Window() = delete;
		MSWin_Window(std::shared_ptr<ConfigManager> configManager);
		~MSWin_Window();

		[[nodiscard]] virtual std::expected<void, std::string> Initialize() override;
	};
}
#endif // defined(Z_WINDOWS)