#pragma once

#include <logger/logger.h>

#include "core/CoreIncludes.h"
#include "math/Size2D.h"
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	using namespace zzz::math;

	class MonitorInfo final : public ISerializable
	{
	public:
		MonitorInfo() = default;
		MonitorInfo(std::string platformMonitorId, std::string name, Size2D<zU32> physicalResolution, Size2D<zU32> logicalResolution,
			zI32 positionX, zI32 positionY, bool isPrimary, float scaleFactor = 1.0f)
			: m_PlatformMonitorId(std::move(platformMonitorId))
			, m_Name(std::move(name))
			, m_PhysicalResolution(physicalResolution)
			, m_LogicalResolution(logicalResolution)
			, m_PositionX(positionX)
			, m_PositionY(positionY)
			, m_IsPrimary(isPrimary)
			, m_ScaleFactor(scaleFactor)
		{}

		MonitorInfo(std::string platformMonitorId, std::string name, Size2D<zU32> resolution,
			zI32 positionX, zI32 positionY, bool isPrimary, float scaleFactor = 1.0f)
			: MonitorInfo(std::move(platformMonitorId), std::move(name), resolution, resolution, positionX, positionY, isPrimary, scaleFactor)
		{}

		[[nodiscard]] const std::string& GetPlatformMonitorId() const noexcept { return m_PlatformMonitorId; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const Size2D<zU32>& GetPhysicalResolution() const noexcept { return m_PhysicalResolution; }
		[[nodiscard]] const Size2D<zU32>& GetLogicalResolution() const noexcept { return m_LogicalResolution; }
		[[nodiscard]] const Size2D<zU32>& GetResolution() const noexcept { return m_LogicalResolution; }
		[[nodiscard]] zI32 GetPositionX() const noexcept { return m_PositionX; }
		[[nodiscard]] zI32 GetPositionY() const noexcept { return m_PositionY; }
		[[nodiscard]] bool IsPrimary() const noexcept { return m_IsPrimary; }
		[[nodiscard]] float GetScaleFactor() const noexcept { return m_ScaleFactor; }

		[[nodiscard]] bool operator==(const MonitorInfo& other) const noexcept
		{
			return m_PlatformMonitorId == other.m_PlatformMonitorId &&
				m_Name == other.m_Name &&
				m_PhysicalResolution == other.m_PhysicalResolution &&
				m_LogicalResolution == other.m_LogicalResolution &&
				m_PositionX == other.m_PositionX &&
				m_PositionY == other.m_PositionY &&
				m_IsPrimary == other.m_IsPrimary &&
				m_ScaleFactor == other.m_ScaleFactor;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[MonitorInfo]", indentation);
			DOut("{}platformMonitorId: {}", nestedIndentation, m_PlatformMonitorId);
			DOut("{}name: {}", nestedIndentation, m_Name);
			DOut("{}physicalResolution: {}x{}", nestedIndentation, m_PhysicalResolution.GetWidth(), m_PhysicalResolution.GetHeight());
			DOut("{}logicalResolution: {}x{}", nestedIndentation, m_LogicalResolution.GetWidth(), m_LogicalResolution.GetHeight());
			DOut("{}position: ({},{})", nestedIndentation, m_PositionX, m_PositionY);
			DOut("{}isPrimary: {}", nestedIndentation, m_IsPrimary);
			DOut("{}scaleFactor: {:.2f} ({}%)", nestedIndentation, m_ScaleFactor, static_cast<int>(m_ScaleFactor * 100.0f + 0.5f));
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_PlatformMonitorId)
				.and_then([&]() { return s.Serialize(buffer, m_Name); })
				.and_then([&]() { return s.Serialize(buffer, m_PhysicalResolution); })
				.and_then([&]() { return s.Serialize(buffer, m_LogicalResolution); })
				.and_then([&]() { return s.Serialize(buffer, m_PositionX); })
				.and_then([&]() { return s.Serialize(buffer, m_PositionY); })
				.and_then([&]() { return s.Serialize(buffer, m_IsPrimary); })
				.and_then([&]() { return s.Serialize(buffer, m_ScaleFactor); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_PlatformMonitorId)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_Name); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_PhysicalResolution); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_LogicalResolution); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_PositionX); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_PositionY); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_IsPrimary); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_ScaleFactor); });
		}

		std::string m_PlatformMonitorId;
		std::string m_Name;
		Size2D<zU32> m_PhysicalResolution{ 1920, 1080 };
		Size2D<zU32> m_LogicalResolution{ 1920, 1080 };
		zI32 m_PositionX{ 0 };
		zI32 m_PositionY{ 0 };
		bool m_IsPrimary{ false };
		float m_ScaleFactor{ 1.0f };
	};
}
