#pragma once

#include "core/utils/Guid.h"
#include "core/hardware/MonitorInfo.h"
#include "core/Serialize/Serializer.h"
#include "math/Rect2D.h"

namespace zzz::core
{
	using namespace zzz::math;

	/**
	 * @brief Сохраненное пользовательское состояние конкретного окна (View) в user.dat.
	 */
	class ViewUserData final : public ISerializable
	{
	public:
		ViewUserData() = default;
		ViewUserData(Guid viewGuid, std::string platformMonitorId, Rect2D<zI32> windowRect, bool isMaximized = false)
			: m_ViewGuid(std::move(viewGuid))
			, m_PlatformMonitorId(std::move(platformMonitorId))
			, m_WindowRect(windowRect)
			, m_IsMaximized(isMaximized)
		{}

		[[nodiscard]] const Guid& GetViewGuid() const noexcept { return m_ViewGuid; }
		[[nodiscard]] const std::string& GetPlatformMonitorId() const noexcept { return m_PlatformMonitorId; }
		[[nodiscard]] const Rect2D<zI32>& GetWindowRect() const noexcept { return m_WindowRect; }
		[[nodiscard]] bool IsMaximized() const noexcept { return m_IsMaximized; }

		void SetPlatformMonitorId(std::string monitorId) noexcept { m_PlatformMonitorId = std::move(monitorId); }
		void SetWindowRect(const Rect2D<zI32>& rect) noexcept { m_WindowRect = rect; }
		void SetIsMaximized(bool isMaximized) noexcept { m_IsMaximized = isMaximized; }

		[[nodiscard]] bool operator==(const ViewUserData& other) const noexcept
		{
			return m_ViewGuid == other.m_ViewGuid &&
				m_PlatformMonitorId == other.m_PlatformMonitorId &&
				m_WindowRect == other.m_WindowRect &&
				m_IsMaximized == other.m_IsMaximized;
		}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[ViewUserData]", indentation);
			DOut("{}viewGuid: {}", nestedIndentation, m_ViewGuid.ToString());
			DOut("{}platformMonitorId: {}", nestedIndentation, m_PlatformMonitorId);
			DOut("{}windowRect: {}", nestedIndentation, m_WindowRect.ToString());
			DOut("{}isMaximized: {}", nestedIndentation, m_IsMaximized);
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_ViewGuid)
				.and_then([&]() { return s.Serialize(buffer, m_PlatformMonitorId); })
				.and_then([&]() { return s.Serialize(buffer, m_WindowRect); })
				.and_then([&]() { return s.Serialize(buffer, m_IsMaximized); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_ViewGuid)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_PlatformMonitorId); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_WindowRect); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_IsMaximized); });
		}

		Guid m_ViewGuid;
		std::string m_PlatformMonitorId;
		Rect2D<zI32> m_WindowRect{ Point2D<zI32>{0, 0}, Size2D<zI32>{1280, 720} };
		bool m_IsMaximized{ false };
	};
}
