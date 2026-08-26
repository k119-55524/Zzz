#pragma once

#include "WindowCommon.h"
#include "../input/Input.h"

namespace zzz::engine
{
	class View;
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

		[[nodiscard]] std::expected<void, std::string> Initialize(const ViewPlatformData& platformData, const View* parentView = nullptr) override;
		[[nodiscard]] bool IsMinimized() const noexcept override { return m_hWnd && ::IsIconic(m_hWnd); }
		[[nodiscard]] bool IsMaximized() const override;
		[[nodiscard]] Rect2D<zI32> GetClientRect() const override;
		void OnMonitorResolutionChanged() override;

		HWND GetHWnd() const noexcept { return m_hWnd; }

		MsgProcResult MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

	private:
		[[nodiscard]] Rect2D<zI32> GetRestoredWindowRect() const;

		HWND m_hWnd;
		MSWinCtx m_Ctx;
	};
}
