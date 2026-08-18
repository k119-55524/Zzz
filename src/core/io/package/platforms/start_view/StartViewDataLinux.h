#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "math/Size2D.h"
#include "math/Point2D.h"
#include "math/Rect2D.h"
#include "core/Serialize/Serializer.h"
#include "core/Enums/eEnumToString.h"
#include "core/Enums/platforms/eLinuxEnums.h"
#include "core/hardware/DisplayMonitorInfo.h"
#include "core/utils/Constants.h"

namespace zzz::core
{
	using namespace zzz::math;

	class StartViewDataLinux final : public ISerializable
	{
	public:
		StartViewDataLinux() = default;
		StartViewDataLinux(std::string title, Size2D<zU32> size, eLinuxWindowMode windowMode = eLinuxWindowMode::Windowed, bool resizable = true, eLinuxDisplayServer displayServer = eLinuxDisplayServer::Auto)
			: title(std::move(title))
			, size(size)
			, windowMode(windowMode)
			, resizable(resizable)
			, displayServer(displayServer)
			, windowRect(Point2D<zI32>{0, 0}, Size2D<zU32>{size.GetWidth(), size.GetHeight()})
		{}

		[[nodiscard]] const std::string& GetTitle() const noexcept { return title; }
		[[nodiscard]] const Size2D<zU32>& GetSize() const noexcept { return size; }
		[[nodiscard]] eLinuxWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }
		[[nodiscard]] eLinuxDisplayServer GetDisplayServer() const noexcept { return displayServer; }
		[[nodiscard]] const Rect2D<zI32>& GetWindowRect() const noexcept { return windowRect; }
		[[nodiscard]] zU32 GetMonitorIndex() const noexcept { return monitorIndex; }

		void SetWindowRect(const Rect2D<zI32>& rect) noexcept { windowRect = rect; }
		void SetMonitorIndex(zU32 index) noexcept { monitorIndex = index; }
		void SetWindowMode(eLinuxWindowMode mode) noexcept { windowMode = mode; }

		void ValidateAndAdjustWindowRect(const std::vector<DisplayMonitorInfo>& monitors)
		{
			if (monitors.empty())
				return;

			if (monitorIndex >= monitors.size())
				monitorIndex = 0;

			const auto& targetMonitor = monitors[monitorIndex];
			zI32 monX = targetMonitor.GetPositionX();
			zI32 monY = targetMonitor.GetPositionY();
			zI32 monW = static_cast<zI32>(targetMonitor.GetResolution().GetWidth());
			zI32 monH = static_cast<zI32>(targetMonitor.GetResolution().GetHeight());

			Rect2D<zI32> monRect(monX, monY, monW, monH);

			zI32 targetW = std::clamp(static_cast<zI32>(windowRect.GetSize().GetWidth()), static_cast<zI32>(c_MinWinSize), monW);
			zI32 targetH = std::clamp(static_cast<zI32>(windowRect.GetSize().GetHeight()), static_cast<zI32>(c_MinWinSize), monH);

			if (!windowRect.Intersects(monRect))
			{
				zI32 x = monX + (monW - targetW) / 2;
				zI32 y = monY + (monH - targetH) / 2;
				windowRect.SetFrom(x, y, targetW, targetH);
				monitorIndex = 0;
			}
			else
			{
				windowRect.SetFrom(windowRect.GetPosition().GetX(), windowRect.GetPosition().GetY(), targetW, targetH);
			}

			size.SetFrom(static_cast<zU32>(targetW), static_cast<zU32>(targetH));
		}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[StartViewDataLinux]", indentation);
			DOut("{}title: {}", nestedIndentation, title);
			DOut("{}size: {}x{}", nestedIndentation, size.GetWidth(), size.GetHeight());
			DOut("{}windowMode: {}", nestedIndentation, EnumToString::ToString(windowMode));
			DOut("{}resizable: {}", nestedIndentation, resizable);
			DOut("{}displayServer: {}", nestedIndentation, EnumToString::ToString(displayServer));
			DOut("{}windowRect: X: {}, Y: {}, W: {}, H: {}", nestedIndentation, windowRect.GetPosition().GetX(), windowRect.GetPosition().GetY(), windowRect.GetSize().GetWidth(), windowRect.GetSize().GetHeight());
			DOut("{}monitorIndex: {}", nestedIndentation, monitorIndex);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, title)
				.and_then([&]() { return s.Serialize(buffer, size); })
				.and_then([&]() { return s.Serialize(buffer, windowMode); })
				.and_then([&]() { return s.Serialize(buffer, resizable); })
				.and_then([&]() { return s.Serialize(buffer, displayServer); })
				.and_then([&]() { return s.Serialize(buffer, windowRect); })
				.and_then([&]() { return s.Serialize(buffer, monitorIndex); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, title)
				.and_then([&]() { return s.Deserialize(buffer, offset, size); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); })
				.and_then([&]() { return s.Deserialize(buffer, offset, displayServer); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowRect); })
				.and_then([&]() { return s.Deserialize(buffer, offset, monitorIndex); });
		}

		std::string title{ "Game Window" };
		Size2D<zU32> size{ 1280, 720 };
		eLinuxWindowMode windowMode = eLinuxWindowMode::Windowed;
		bool resizable = true;
		eLinuxDisplayServer displayServer = eLinuxDisplayServer::Auto;
		Rect2D<zI32> windowRect{ Point2D<zI32>{0, 0}, Size2D<zU32>{1280, 720} };
		zU32 monitorIndex{ 0 };
	};
}
