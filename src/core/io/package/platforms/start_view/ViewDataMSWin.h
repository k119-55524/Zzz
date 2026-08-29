#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "math/Size2D.h"
#include "math/Point2D.h"
#include "math/Rect2D.h"
#include "core/Serialize/Serializer.h"
#include "core/Enums/eEnumToString.h"
#include "core/Enums/platforms/eMSWinEnums.h"
#include "core/hardware/MonitorInfo.h"
#include "core/constants/DisplayConstants.h"
#include "core/constants/LogCategoryConstants.h"
#include <logger/logger.h>

#include "core/enums/eWindowState.h"

namespace zzz::core
{
	using namespace zzz::math;

	class ViewDataMSWin final : public ISerializable
	{
	public:
		ViewDataMSWin() = default;
		ViewDataMSWin(std::string title, Size2D<zU32> size, eMSWinWindowMode windowMode = eMSWinWindowMode::Windowed, bool resizable = true, eWindowState windowState = eWindowState::Normal)
			: title(std::move(title))
			, size(size)
			, windowMode(windowMode)
			, resizable(resizable)
			, windowRect(Point2D<zI32>{0, 0}, size)
			, windowState(windowState)
		{}

		[[nodiscard]] const std::string& GetTitle() const noexcept { return title; }
		[[nodiscard]] const Size2D<zU32>& GetSize() const noexcept { return size; }
		[[nodiscard]] eMSWinWindowMode GetWindowMode() const noexcept { return windowMode; }
		[[nodiscard]] bool IsResizable() const noexcept { return resizable; }
		[[nodiscard]] const Rect2D<zI32>& GetWindowRect() const noexcept { return windowRect; }
		[[nodiscard]] zU32 GetMonitorIndex() const noexcept { return monitorIndex; }
		[[nodiscard]] const std::string& GetMonitorId() const noexcept { return monitorId; }
		[[nodiscard]] eWindowState GetWindowState() const noexcept { return windowState; }

		void SetWindowRect(const Rect2D<zI32>& rect) noexcept { windowRect = rect; }
		void SetMonitorIndex(zU32 index) noexcept { monitorIndex = index; }
		void SetMonitorId(std::string id) noexcept { monitorId = std::move(id); }
		void SetWindowMode(eMSWinWindowMode mode) noexcept { windowMode = mode; }
		void SetWindowState(eWindowState state) noexcept { windowState = state; }

		void ValidateAndAdjustWindowRect(const std::vector<MonitorInfo>& monitors)
		{
			DOut(::zzz::core::Assets, "[ViewDataMSWin::ValidateAndAdjustWindowRect] Начало валидации окна. Текущий windowRect: {}, monitorIndex: {}", windowRect.ToString(), monitorIndex);

			if (monitors.empty())
			{
				DOut(::zzz::core::Assets, "[ViewDataMSWin::ValidateAndAdjustWindowRect] Список мониторов пуст. Пропуск валидации.");
				return;
			}

			if (monitorIndex >= monitors.size())
			{
				DOut(::zzz::core::Assets, "[ViewDataMSWin::ValidateAndAdjustWindowRect] Предупреждение: monitorIndex ({}) за пределами кол-ва мониторов ({}). Сброс на 0.", monitorIndex, monitors.size());
				monitorIndex = 0;
			}

			const auto& mon = monitors[monitorIndex];
			Rect2D<zI32> monRect(
				Point2D<zI32>{ mon.GetPositionX(), mon.GetPositionY() },
				Size2D<zI32>{ static_cast<zI32>(mon.GetResolution().GetWidth()), static_cast<zI32>(mon.GetResolution().GetHeight()) }
			);

			DOut(::zzz::core::Assets, "[ViewDataMSWin::ValidateAndAdjustWindowRect] Целевой монитор #{}: {}", monitorIndex, monRect.ToString());

			zI32 targetW = std::clamp<zI32>(windowRect.GetSize().GetWidth(), c_MinWinSize, monRect.GetSize().GetWidth());
			zI32 targetH = std::clamp<zI32>(windowRect.GetSize().GetHeight(), c_MinWinSize, monRect.GetSize().GetHeight());

			if (static_cast<zU32>(targetW) != windowRect.GetSize().GetWidth() || static_cast<zU32>(targetH) != windowRect.GetSize().GetHeight())
			{
				DOut(::zzz::core::Assets, "[ViewDataMSWin::ValidateAndAdjustWindowRect] Размер окна скорректирован под монитор/c_MinWinSize: {}x{}", targetW, targetH);
				windowRect.SetSize(Size2D<zU32>{ static_cast<zU32>(targetW), static_cast<zU32>(targetH) });
			}

			if (!monRect.Intersects(windowRect))
			{
				DOut(::zzz::core::Assets, "[ViewDataMSWin::ValidateAndAdjustWindowRect] Внимание! Окно не пересекается с монитором #{}! Перенос по центру.", monitorIndex);
				zI32 centerX = monRect.GetPosition().GetX() + (monRect.GetSize().GetWidth() - windowRect.GetSize().GetWidth()) / 2;
				zI32 centerY = monRect.GetPosition().GetY() + (monRect.GetSize().GetHeight() - windowRect.GetSize().GetHeight()) / 2;
				windowRect.SetPosition(Point2D<zI32>{ centerX, centerY });
			}

			DOut(::zzz::core::Assets, "[ViewDataMSWin::ValidateAndAdjustWindowRect] Итоговый валидированный windowRect: {}", windowRect.ToString());
		}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Assets, "{}[ViewDataMSWin]", indentation);
			DOut(::zzz::core::Assets, "{}title: {}", nestedIndentation, title);
			DOut(::zzz::core::Assets, "{}size: {}x{}", nestedIndentation, size.GetWidth(), size.GetHeight());
			DOut(::zzz::core::Assets, "{}windowMode: {}", nestedIndentation, EnumToString::ToString(windowMode));
			DOut(::zzz::core::Assets, "{}windowState: {}", nestedIndentation, EnumToString::ToString(windowState));
			DOut(::zzz::core::Assets, "{}resizable: {}", nestedIndentation, resizable ? "true" : "false");
			DOut(::zzz::core::Assets, "{}windowRect: {}", nestedIndentation, windowRect.ToString());
			DOut(::zzz::core::Assets, "{}monitorIndex: {}", nestedIndentation, monitorIndex);
			DOut(::zzz::core::Assets, "{}monitorId: {}", nestedIndentation, monitorId);
#endif
		}

		[[nodiscard]] bool operator==(const ViewDataMSWin& other) const noexcept
		{
			return title == other.title &&
				size == other.size &&
				windowMode == other.windowMode &&
				resizable == other.resizable &&
				windowRect == other.windowRect &&
				monitorIndex == other.monitorIndex &&
				monitorId == other.monitorId &&
				windowState == other.windowState;
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, title)
				.and_then([&]() { return s.Serialize(buffer, size); })
				.and_then([&]() { return s.Serialize(buffer, windowMode); })
				.and_then([&]() { return s.Serialize(buffer, resizable); })
				.and_then([&]() { return s.Serialize(buffer, windowRect); })
				.and_then([&]() { return s.Serialize(buffer, monitorIndex); })
				.and_then([&]() { return s.Serialize(buffer, monitorId); })
				.and_then([&]() { return s.Serialize(buffer, windowState); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, title)
				.and_then([&]() { return s.Deserialize(buffer, offset, size); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowMode); })
				.and_then([&]() { return s.Deserialize(buffer, offset, resizable); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowRect); })
				.and_then([&]() { return s.Deserialize(buffer, offset, monitorIndex); })
				.and_then([&]() { return s.Deserialize(buffer, offset, monitorId); })
				.and_then([&]() { return s.Deserialize(buffer, offset, windowState); });
		}

		std::string title{ "MSWin Application Window" };
		Size2D<zU32> size{ 1280, 720 };
		eMSWinWindowMode windowMode{ eMSWinWindowMode::Windowed };
		bool resizable{ true };
		Rect2D<zI32> windowRect{ Point2D<zI32>{0, 0}, Size2D<zI32>{1280, 720} };
		zU32 monitorIndex{ 0 };
		std::string monitorId;
		eWindowState windowState{ eWindowState::Normal };
	};
}
