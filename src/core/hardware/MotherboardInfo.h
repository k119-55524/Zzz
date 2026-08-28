#pragma once

#include <logger/logger.h>

#include "core/CoreIncludes.h"
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class MotherboardInfo final : public ISerializable
	{
	public:
		MotherboardInfo() = default;
		MotherboardInfo(std::string vendor, std::string model, std::string systemUuid)
			: m_Vendor(std::move(vendor))
			, m_Model(std::move(model))
			, m_SystemUuid(std::move(systemUuid))
		{}

		[[nodiscard]] const std::string& GetVendor() const noexcept { return m_Vendor; }
		[[nodiscard]] const std::string& GetModel() const noexcept { return m_Model; }
		[[nodiscard]] const std::string& GetSystemUuid() const noexcept { return m_SystemUuid; }

		[[nodiscard]] bool operator==(const MotherboardInfo& other) const noexcept
		{
			return m_Vendor == other.m_Vendor &&
				m_Model == other.m_Model &&
				m_SystemUuid == other.m_SystemUuid;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[MotherboardInfo]", indentation);
			DOut("{}vendor: {}", nestedIndentation, m_Vendor);
			DOut("{}model: {}", nestedIndentation, m_Model);
			DOut("{}systemUuid: {}", nestedIndentation, m_SystemUuid);
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_Vendor)
				.and_then([&]() { return s.Serialize(buffer, m_Model); })
				.and_then([&]() { return s.Serialize(buffer, m_SystemUuid); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_Vendor)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_Model); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_SystemUuid); });
		}

		std::string m_Vendor;
		std::string m_Model;
		std::string m_SystemUuid;
	};
}
