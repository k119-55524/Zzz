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
		WinMSWindows(const std::shared_ptr<IPlatform> platform, const std::shared_ptr<IInput> input);
		~WinMSWindows() override;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize(const std::string_view appName) override;
		LRESULT MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		private:
			HWND m_hWnd;
	};
}
#endif // defined(Z_WINDOWS)