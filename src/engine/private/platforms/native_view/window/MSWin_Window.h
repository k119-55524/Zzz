#pragma once

#if defined(Z_WINDOWS)

#include "../../../../header.h"
#include "../../../core/config/EngineConfig.h"

#include "IWindow.h"

namespace zzz::engine
{
	class MSWin_Window final : public IWindow
	{
	public:
		MSWin_Window() = delete;
		MSWin_Window(const EngineConfig& config);
		~MSWin_Window();

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;

		private:
			static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

			HWND m_hWnd;
	};
}
#endif // defined(Z_WINDOWS)