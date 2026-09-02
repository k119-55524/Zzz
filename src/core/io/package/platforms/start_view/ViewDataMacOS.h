#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "math/Math.h"
#include "core/Serialize/Serializer.h"
#include "core/Enums/platforms/eMacOSEnums.h"
#include "core/hardware/MonitorInfo.h"
#include "core/constants/DisplayConstants.h"

namespace zzz::core
{
	using namespace zzz::math;

	class ViewDataMacOS final : public ISerializable
	{
	public:
		ViewDataMacOS() : isPrimary(false) {}
		ViewDataMacOS(std::string title, Size2D<zU32> size, eMacOSWindowMode windowMode = eMacOSWindowMode::Windowed, bool resizable = true)
			: title(std::move(title))
			, size(size)
			, windowMode(windowMode)
			, resizable(resizable)
			, windowRect(Point2D<zI32>{0, 0}, Size2D<zU32>{size.width, size.height})
			, isPrimary(false)
		{}

		[[nodiscard]] const std::string& GetTitle() const noexcept { return title; }
		[[nodiscard]] const Size2D<zU32>& GetSize() const noexcept { return size; }
		[[nodiscard]] eMacOSWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }
		[[nodiscard]] const Rect2D<zI32>& GetWindowRect() const noexcept { return windowRect; }
		[[nodiscard]] zU32 GetMonitorIndex() const noexcept { return monitorIndex; }
		[[nodiscard]] bool IsPrimary() const noexcept { return isPrimary; }

		void SetWindowRect(const Rect2D<zI32>& rect) noexcept { windowRect = rect; }
		void SetMonitorIndex(zU32 index) noexcept { monitorIndex = index; }
		void SetWindowMode(eMacOSWindowMode mode) noexcept { windowMode = mode; }
		void SetPrimary(bool primary) noexcept { isPrimary = primary; }

		void ValidateAndAdjustWindowRect(const std::vector<MonitorInfo>& monitors)
		{
			if (monitors.empty())
				return;

			if (monitorIndex >= monitors.size())
				monitorIndex = 0;

			const auto& targetMonitor = monitors[monitorIndex];
			zI32 monX = targetMonitor.GetPositionX();
			zI32 monY = targetMonitor.GetPositionY();
			zI32 monW = static_cast<zI32>(targetMonitor.GetResolution().width);
			zI32 monH = static_cast<zI32>(targetMonitor.GetResolution().height);

			Rect2D<zI32> monRect(monX, monY, monW, monH);

			zI32 targetW = std::clamp(static_cast<zI32>(windowRect.size.width), static_cast<zI32>(c_MinWinSize), monW);
			zI32 targetH = std::clamp(static_cast<zI32>(windowRect.size.height), static_cast<zI32>(c_MinWinSize), monH);

			if (!windowRect.Intersects(monRect))
			{
				zI32 x = monX + (monW - targetW) / 2;
				zI32 y = monY + (monH - targetH) / 2;
				windowRect.SetFrom(x, y, targetW, targetH);
				monitorIndex = 0;
			}
			else
			{
				windowRect.SetFrom(windowRect.position.x, windowRect.position.y, targetW, targetH);
			}

			size.SetFrom(static_cast<zU32>(targetW), static_cast<zU32>(targetH));
		}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[StartViewDataMacOS]", indentation);
			DOut(::zzz::core::Assets, "{}title: {}", nestedIndentation, title);
			DOut(::zzz::core::Assets, "{}size: {}x{}", nestedIndentation, size.width, size.height);
			DOut(::zzz::core::Assets, "{}windowMode: {}", nestedIndentation, ToString(windowMode));
			DOut(::zzz::core::Assets, "{}resizable: {}", nestedIndentation, resizable);
			DOut(::zzz::core::Assets, "{}windowRect: X: {}, Y: {}, W: {}, H: {}", nestedIndentation, windowRect.position.x, windowRect.position.y, windowRect.size.width, windowRect.size.height);
			DOut(::zzz::core::Assets, "{}monitorIndex: {}", nestedIndentation, monitorIndex);
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, title)
				.and_then([&]() { return s.Serialize(buffer, size); })
				.and_then([&]() { return s.Serialize(buffer, windowMode); })
				.and_then([&]() { return s.Serialize(buffer, resizable); })
				.and_then([&]() { return s.Serialize(buffer, windowRect); })
				.and_then([&]() { return s.Serialize(buffer, monitorIndex); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, title)
				.and_then([&]() { return s.Deserialize(buffer, offset, size); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowRect); })
				.and_then([&]() { return s.Deserialize(buffer, offset, monitorIndex); });
		}

		std::string title{ "Game Window" };
		Size2D<zU32> size{ 1280, 720 };
		eMacOSWindowMode windowMode = eMacOSWindowMode::Windowed;
		bool resizable = true;
		Rect2D<zI32> windowRect{ Point2D<zI32>{0, 0}, Size2D<zU32>{1280, 720} };
		zU32 monitorIndex{ 0 };

		// Не сериализуется - транзиентная метка "кто создаёт это окно" (Primary или нет).
		// Проставляется заново каждый раз в UserSettingsManager::GetOrCreateXxxViewPlatformData()
		// при чтении/создании и не должна переживать сериализацию в user.dat.
		bool isPrimary;
	};
}
