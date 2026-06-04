#pragma once

#if defined(Z_WINDOWS)

#include "../../../../header.h"

#include "IWindow.h"

namespace zzz::engine
{
	class MSWin_Window final : public IWindow
	{
	public:
		MSWin_Window() = delete;
		MSWin_Window(const PlatformConfig& platformConfig);
		~MSWin_Window();

		[[nodiscard]] virtual std::expected<void, std::string> Initialize() override;

		private:
			static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
	};
}
#endif // defined(Z_WINDOWS)