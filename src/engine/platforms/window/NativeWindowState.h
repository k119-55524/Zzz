#pragma once

#include <span>
#include <string>
#include <vector>

#include "core/enums/eWindowState.h"
#include "core/Serialize/Serializer.h"

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::engine
{
	class NativeWindowState final : public ISerializable
	{
	public:
		NativeWindowState() = default;
		NativeWindowState(std::string monitorId, Rect2D<zI32> windowRect, eWindowState state)
			: m_MonitorId(std::move(monitorId))
			, m_WindowRect(windowRect)
			, m_State(state)
		{}

		[[nodiscard]] const std::string& GetMonitorId() const noexcept { return m_MonitorId; }
		[[nodiscard]] const Rect2D<zI32>& GetWindowRect() const noexcept { return m_WindowRect; }
		[[nodiscard]] eWindowState GetState() const noexcept { return m_State; }

		void SetMonitorId(std::string monitorId) noexcept { m_MonitorId = std::move(monitorId); }
		void SetWindowRect(const Rect2D<zI32>& rect) noexcept { m_WindowRect = rect; }
		void SetState(eWindowState state) noexcept { m_State = state; }

		[[nodiscard]] bool operator==(const NativeWindowState& other) const noexcept
		{
			return m_MonitorId == other.m_MonitorId &&
				m_WindowRect == other.m_WindowRect &&
				m_State == other.m_State;
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_MonitorId)
				.and_then([&]() { return s.Serialize(buffer, m_WindowRect); })
				.and_then([&]() { return s.Serialize(buffer, m_State); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_MonitorId)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_WindowRect); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_State); });
		}

		std::string m_MonitorId;
		Rect2D<zI32> m_WindowRect{ Point2D<zI32>{0, 0}, Size2D<zI32>{1280, 720} };
		eWindowState m_State{ eWindowState::Normal };
	};
}
