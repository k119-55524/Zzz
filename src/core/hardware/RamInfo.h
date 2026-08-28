#pragma once

#include <logger/logger.h>

#include "core/CoreIncludes.h"
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	enum class eRamType : zU8
	{
		Unknown = 0,
		DDR,
		DDR2,
		DDR3,
		DDR4,
		DDR5,
		LPDDR3,
		LPDDR4,
		LPDDR5
	};

	class RamInfo final : public ISerializable
	{
	public:
		RamInfo() = default;
		RamInfo(zU64 totalRamBytes, zU64 availableRamBytes, eRamType type = eRamType::Unknown, zU32 speedMTs = 0)
			: m_TotalRamBytes(totalRamBytes)
			, m_AvailableRamBytes(availableRamBytes)
			, m_Type(type)
			, m_SpeedMTs(speedMTs)
		{}

		[[nodiscard]] zU64 GetTotalRamBytes() const noexcept { return m_TotalRamBytes; }
		[[nodiscard]] zU64 GetAvailableRamBytes() const noexcept { return m_AvailableRamBytes; }
		[[nodiscard]] eRamType GetType() const noexcept { return m_Type; }
		[[nodiscard]] zU32 GetSpeedMTs() const noexcept { return m_SpeedMTs; }

		[[nodiscard]] bool operator==(const RamInfo& other) const noexcept
		{
			return m_TotalRamBytes == other.m_TotalRamBytes &&
				m_Type == other.m_Type &&
				m_SpeedMTs == other.m_SpeedMTs;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[RamInfo]", indentation);
			DOut("{}type: {}", nestedIndentation, GetTypeString(m_Type));
			DOut("{}speedMTs: {} MT/s", nestedIndentation, m_SpeedMTs);
			DOut("{}totalRamBytes: {} MB", nestedIndentation, m_TotalRamBytes / (1024 * 1024));
			DOut("{}availableRamBytes: {} MB", nestedIndentation, m_AvailableRamBytes / (1024 * 1024));
#endif
		}

	private:
		[[nodiscard]] static constexpr std::string_view GetTypeString(eRamType type) noexcept
		{
			switch (type)
			{
			case eRamType::DDR:    return "DDR";
			case eRamType::DDR2:   return "DDR2";
			case eRamType::DDR3:   return "DDR3";
			case eRamType::DDR4:   return "DDR4";
			case eRamType::DDR5:   return "DDR5";
			case eRamType::LPDDR3: return "LPDDR3";
			case eRamType::LPDDR4: return "LPDDR4";
			case eRamType::LPDDR5: return "LPDDR5";
			default: return "Unknown";
			}
		}

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_TotalRamBytes)
				.and_then([&]() { return s.Serialize(buffer, m_AvailableRamBytes); })
				.and_then([&]() { return s.Serialize(buffer, static_cast<zU8>(m_Type)); })
				.and_then([&]() { return s.Serialize(buffer, m_SpeedMTs); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			zU8 rawType = 0;
			return s.Deserialize(buffer, offset, m_TotalRamBytes)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_AvailableRamBytes); })
				.and_then([&]() { return s.Deserialize(buffer, offset, rawType); })
				.and_then([&]() { m_Type = static_cast<eRamType>(rawType); return s.Deserialize(buffer, offset, m_SpeedMTs); });
		}

		zU64 m_TotalRamBytes{ 0 };
		zU64 m_AvailableRamBytes{ 0 };
		eRamType m_Type{ eRamType::Unknown };
		zU32 m_SpeedMTs{ 0 };
	};
}
