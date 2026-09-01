#pragma once


#include "WindowCommon.h"

#include "../../header.h"

namespace zzz::engine
{
	class View;

	class WinAndroid final : public WindowBase
	{
	public:
		struct AndroidActinityCtx
		{
			WinAndroid*	window;
			Input*		input;
		};

		WinAndroid() = delete;
		WinAndroid(const Platform& platform, const std::shared_ptr<Input> input, WindowCallbacks callbacks);
		~WinAndroid() override;

		[[nodiscard]] std::expected<void, std::string> Initialize(const ViewPlatformData& windowSettings, const View* parentView = nullptr) override;
		[[nodiscard]] bool IsMinimized() const noexcept override { return false; }
		[[nodiscard]] bool IsMaximized() const override { return true; }
		[[nodiscard]] Rect2D<zI32> GetClientRect() const override { return Rect2D<zI32>{ Point2D<zI32>{0, 0}, Size2D<zI32>{static_cast<zI32>(m_WinSize.width), static_cast<zI32>(m_WinSize.height)} }; }
		void OnMonitorResolutionChanged() override {}

		void ProcessAppCmd(int32_t cmd);

	private:
		AndroidActinityCtx m_Ctx;
	};
}
