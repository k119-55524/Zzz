#pragma once

#if defined(Z_WINDOWS)

#include "IPlatform.h"

namespace zzz::engine
{
	class PlatformMSWindows final : public IPlatform
	{
	public:
		PlatformMSWindows(std::string_view appName, std::shared_ptr<void> platformData = nullptr);
		~PlatformMSWindows() override;

	private:
		void InitializeImpl() override;
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;
	};
}
#endif // defined(Z_WINDOWS)