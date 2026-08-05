#pragma once

#include "WindowCommon.h"
#include "../input/Input.h"

namespace zzz::engine
{
	class InputMSWindows;

	class WinMSWindows final : public WindowBase
	{
	public:
		struct MSWinCtx
		{
			WinMSWindows*	window;
			Input*			input;
		};

		struct MsgProcResult
		{
			bool	isContinue;
			LRESULT	result;
		};

		WinMSWindows() = delete;
		WinMSWindows(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinMSWindows();

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::string_view appName, void* data = nullptr);
		HWND GetHWnd() const noexcept { return m_hWnd; }
		MsgProcResult MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

		private:
			HWND m_hWnd;
			MSWinCtx m_Ctx;

			bool m_IsMinimized;
	};
}
