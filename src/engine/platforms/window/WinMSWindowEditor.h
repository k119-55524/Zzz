#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

#include "WindowCommon.h"
#include "../input/Input.h"

#if defined(Z_WINDOWS)
#include <windows.h>
#endif

namespace zzz::engine
{
	class View;
	class InputEditor;

	class WinMSWindowEditor final : public WindowBase
	{
	public:
		WinMSWindowEditor() = delete;
		WinMSWindowEditor(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinMSWindowEditor();

		[[nodiscard]] std::expected<void, std::string> Initialize(const ViewPlatformData& settings, const View* parentView = nullptr) override;
		[[nodiscard]] bool IsMinimized() const noexcept override { return false; }
		[[nodiscard]] bool IsMaximized() const override { return false; }
		[[nodiscard]] Rect2D<zI32> GetClientRect() const override { return Rect2D<zI32>{ Point2D<zI32>{0, 0}, Size2D<zI32>{static_cast<zI32>(m_WinSize.width), static_cast<zI32>(m_WinSize.height)} }; }
		void OnMonitorResolutionChanged() override {}

		HWND GetHWnd() const noexcept { return m_hWnd; }

	private:
		HWND m_hWnd = nullptr;
	};
}

#endif // defined(Z_EDITOR)
