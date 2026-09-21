#pragma once

#include "core/CoreIncludes.h"
#include "core/logger/logger.h"
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class CpuInfo final : public ISerializable
	{
	public:
		CpuInfo() = default;
		CpuInfo(std::string name, std::string architecture, zU32 physicalCores, zU32 logicalCores, zU32 baseClockMHz)
			: m_Name(std::move(name))
			, m_Architecture(std::move(architecture))
			, m_PhysicalCores(physicalCores)
			, m_LogicalCores(logicalCores)
			, m_BaseClockMHz(baseClockMHz)
		{}

		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const std::string& GetArchitecture() const noexcept { return m_Architecture; }
		[[nodiscard]] zU32 GetPhysicalCores() const noexcept { return m_PhysicalCores; }
		[[nodiscard]] zU32 GetLogicalCores() const noexcept { return m_LogicalCores; }
		[[nodiscard]] zU32 GetBaseClockMHz() const noexcept { return m_BaseClockMHz; }

		[[nodiscard]] bool operator==(const CpuInfo& other) const noexcept
		{
			return m_Name == other.m_Name &&
				m_Architecture == other.m_Architecture &&
				m_PhysicalCores == other.m_PhysicalCores &&
				m_LogicalCores == other.m_LogicalCores;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut(::zzz::core::Hardware, "{}[CpuInfo]", indentation);
			DOut(::zzz::core::Hardware, "{}name: {}", nestedIndentation, m_Name);
			DOut(::zzz::core::Hardware, "{}arch: {}", nestedIndentation, m_Architecture);
			DOut(::zzz::core::Hardware, "{}physicalCores: {}", nestedIndentation, m_PhysicalCores);
			DOut(::zzz::core::Hardware, "{}logicalCores: {}", nestedIndentation, m_LogicalCores);
			DOut(::zzz::core::Hardware, "{}baseClockMHz: {}", nestedIndentation, m_BaseClockMHz);
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_Name)
				.and_then([&]() { return s.Serialize(buffer, m_Architecture); })
				.and_then([&]() { return s.Serialize(buffer, m_PhysicalCores); })
				.and_then([&]() { return s.Serialize(buffer, m_LogicalCores); })
				.and_then([&]() { return s.Serialize(buffer, m_BaseClockMHz); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			return s.Deserialize(buffer, offset, m_Name)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_Architecture); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_PhysicalCores); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_LogicalCores); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_BaseClockMHz); });
		}

		std::string m_Name;
		std::string m_Architecture;
		zU32 m_PhysicalCores{ 0 };
		zU32 m_LogicalCores{ 0 };
		zU32 m_BaseClockMHz{ 0 };
	};
}
