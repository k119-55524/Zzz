#pragma once


#include "Window_Common.h"
#include "../../header.h"

namespace zzz::engine
{
	class View;

	class WinMacOS final : public WindowBase
	{
	public:
		WinMacOS() = delete;
		WinMacOS(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinMacOS() override;

		[[nodiscard]] std::expected<void, std::string> Initialize(const ViewPlatformData& windowSettings, const View* parentView = nullptr) override;
		[[nodiscard]] bool IsMinimized() const noexcept override { return false; }
		[[nodiscard]] bool IsMaximized() const override { return false; }
		[[nodiscard]] Rect2D<zI32> GetClientRect() const override { return Rect2D<zI32>{ Point2D<zI32>{0, 0}, Size2D<zI32>{static_cast<zI32>(m_WinSize.GetWidth()), static_cast<zI32>(m_WinSize.GetHeight())} }; }
		void OnMonitorResolutionChanged() override {}
	};
}
