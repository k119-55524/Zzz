#pragma once

#if defined(Z_WINDOWS)

#include "../../../../header.h"
#include "../../../core/config/EngineConfig.h"

#include "IWindow.h"

namespace zzz::engine
{
	class MSWindows final : public IWindow
	{
	public:
		MSWindows() = delete;
		MSWindows(const EngineConfig& config);
		~MSWindows() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;

		private:
			static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
			LRESULT MsgProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

			HWND m_hWnd;
	};
}
#endif // defined(Z_WINDOWS)