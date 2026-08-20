#pragma once

#include <string>
#include <span>
#include <vector>
#include "math/Rect2D.h"
#include "core/enums/eWindowState.h"
#include "core/Serialize/Serializer.h"

namespace zzz::engine
{
	class NativeWindowState final : public zzz::core::ISerializable
	{
	public:
		NativeWindowState() = default;
		NativeWindowState(std::string monitorId, zzz::math::Rect2D<zI32> windowRect, zzz::core::eWindowState state)
			: m_MonitorId(std::move(monitorId))
			, m_WindowRect(windowRect)
			, m_State(state)
		{}

		[[nodiscard]] const std::string& GetMonitorId() const noexcept { return m_MonitorId; }
		[[nodiscard]] const zzz::math::Rect2D<zI32>& GetWindowRect() const noexcept { return m_WindowRect; }
		[[nodiscard]] zzz::core::eWindowState GetState() const noexcept { return m_State; }

		void SetMonitorId(std::string monitorId) noexcept { m_MonitorId = std::move(monitorId); }
		void SetWindowRect(const zzz::math::Rect2D<zI32>& rect) noexcept { m_WindowRect = rect; }
		void SetState(zzz::core::eWindowState state) noexcept { m_State = state; }

		[[nodiscard]] bool operator==(const NativeWindowState& other) const noexcept
		{
			return m_MonitorId == other.m_MonitorId &&
				m_WindowRect == other.m_WindowRect &&
				m_State == other.m_State;
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const zzz::core::Serializer& s) const override
		{
			return s.Serialize(buffer, m_MonitorId)
				.and_then([&]() { return s.Serialize(buffer, m_WindowRect); })
				.and_then([&]() { return s.Serialize(buffer, m_State); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const zzz::core::Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_MonitorId)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_WindowRect); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_State); });
		}

		std::string m_MonitorId;
		zzz::math::Rect2D<zI32> m_WindowRect{ zzz::math::Point2D<zI32>{0, 0}, zzz::math::Size2D<zI32>{1280, 720} };
		zzz::core::eWindowState m_State{ zzz::core::eWindowState::Normal };
	};
}
