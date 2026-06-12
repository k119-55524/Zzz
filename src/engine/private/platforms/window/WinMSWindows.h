#pragma once

#include "../../../header.h"
#include "../config/EngineConfig.h"

#include "Window_Common.h"
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
		WinMSWindows(const std::shared_ptr<Platform> platform, const std::shared_ptr<Input> input, std::function<void()> onWindowClose);
		~WinMSWindows();

		[[nodiscard]] std::expected<void, std::string> Initialize(const std::string_view appName);
		MsgProcResult MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		private:
			HWND m_hWnd;
			MSWinCtx m_Ctx;

			bool IsMinimized;
	};
}
