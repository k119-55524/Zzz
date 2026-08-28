#pragma once

#include <logger/logger.h>

#include "core/CoreIncludes.h"
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class NetworkAdapterInfo final : public ISerializable
	{
	public:
		NetworkAdapterInfo() = default;
		NetworkAdapterInfo(std::string name, std::string macAddress, zU64 transmitSpeedMbps, zU64 receiveSpeedMbps, bool isActive)
			: m_Name(std::move(name))
			, m_MacAddress(std::move(macAddress))
			, m_TransmitSpeedMbps(transmitSpeedMbps)
			, m_ReceiveSpeedMbps(receiveSpeedMbps)
			, m_IsActive(isActive)
		{}

		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const std::string& GetMacAddress() const noexcept { return m_MacAddress; }
		[[nodiscard]] zU64 GetTransmitSpeedMbps() const noexcept { return m_TransmitSpeedMbps; }
		[[nodiscard]] zU64 GetReceiveSpeedMbps() const noexcept { return m_ReceiveSpeedMbps; }
		[[nodiscard]] bool IsActive() const noexcept { return m_IsActive; }

		[[nodiscard]] bool operator==(const NetworkAdapterInfo& other) const noexcept
		{
			return m_Name == other.m_Name &&
				m_MacAddress == other.m_MacAddress;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[NetworkAdapterInfo]", indentation);
			DOut("{}name: {}", nestedIndentation, m_Name);
			DOut("{}macAddress: {}", nestedIndentation, m_MacAddress);
			DOut("{}transmitSpeedMbps: {}", nestedIndentation, m_TransmitSpeedMbps);
			DOut("{}receiveSpeedMbps: {}", nestedIndentation, m_ReceiveSpeedMbps);
			DOut("{}isActive: {}", nestedIndentation, m_IsActive);
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_Name)
				.and_then([&]() { return s.Serialize(buffer, m_MacAddress); })
				.and_then([&]() { return s.Serialize(buffer, m_TransmitSpeedMbps); })
				.and_then([&]() { return s.Serialize(buffer, m_ReceiveSpeedMbps); })
				.and_then([&]() { return s.Serialize(buffer, m_IsActive); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_Name)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_MacAddress); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_TransmitSpeedMbps); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_ReceiveSpeedMbps); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_IsActive); });
		}

		std::string m_Name;
		std::string m_MacAddress;
		zU64 m_TransmitSpeedMbps{ 0 };
		zU64 m_ReceiveSpeedMbps{ 0 };
		bool m_IsActive{ false };
	};
}
