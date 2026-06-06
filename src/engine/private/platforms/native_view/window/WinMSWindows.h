#pragma once

#if defined(Z_WINDOWS)

#include "../../../../header.h"
#include "../../../core/config/EngineConfig.h"

#include "IWindow.h"

namespace zzz::engine
{
	class WinMSWindows final : public IWindow
	{
	public:
		WinMSWindows() = delete;
		WinMSWindows(const std::shared_ptr<IPlatform> platform);
		~WinMSWindows() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;

		private:
			static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
			LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

			static size_t s_WindowCount;

			HWND m_hWnd;
	};
}
#endif // defined(Z_WINDOWS)