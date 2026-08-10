#pragma once

#include <logger/logger.h>

#include "core/CoreIncludes.h"
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	enum class eStorageType : zU8
	{
		Unknown = 0,
		HDD,
		SSD,
		NVMe,
		Removable,
		CDROM,
		RAMDisk,
		Network
	};

	class StorageInfo final : public ISerializable
	{
	public:
		StorageInfo() = default;
		StorageInfo(std::string name, std::string mountPath, zU64 totalSizeBytes, zU64 freeSizeBytes, bool isSystemDrive, eStorageType type = eStorageType::Unknown)
			: m_Name(std::move(name))
			, m_MountPath(std::move(mountPath))
			, m_TotalSizeBytes(totalSizeBytes)
			, m_FreeSizeBytes(freeSizeBytes)
			, m_IsSystemDrive(isSystemDrive)
			, m_Type(type)
		{}

		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] const std::string& GetMountPath() const noexcept { return m_MountPath; }
		[[nodiscard]] zU64 GetTotalSizeBytes() const noexcept { return m_TotalSizeBytes; }
		[[nodiscard]] zU64 GetFreeSizeBytes() const noexcept { return m_FreeSizeBytes; }
		[[nodiscard]] bool IsSystemDrive() const noexcept { return m_IsSystemDrive; }
		[[nodiscard]] eStorageType GetType() const noexcept { return m_Type; }

		[[nodiscard]] bool operator==(const StorageInfo& other) const noexcept
		{
			return m_Name == other.m_Name &&
				m_MountPath == other.m_MountPath &&
				m_TotalSizeBytes == other.m_TotalSizeBytes &&
				m_IsSystemDrive == other.m_IsSystemDrive &&
				m_Type == other.m_Type;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[StorageInfo]", indentation);
			DOut("{}name: {}", nestedIndentation, m_Name);
			DOut("{}mountPath: {}", nestedIndentation, m_MountPath);
			DOut("{}type: {}", nestedIndentation, GetTypeString(m_Type));
			DOut("{}totalSizeBytes: {} GB", nestedIndentation, m_TotalSizeBytes / (1024 * 1024 * 1024));
			DOut("{}freeSizeBytes: {} GB", nestedIndentation, m_FreeSizeBytes / (1024 * 1024 * 1024));
			DOut("{}isSystemDrive: {}", nestedIndentation, m_IsSystemDrive);
#endif
		}

	private:
		[[nodiscard]] static constexpr std::string_view GetTypeString(eStorageType type) noexcept
		{
			switch (type)
			{
			case eStorageType::HDD: return "HDD";
			case eStorageType::SSD: return "SSD";
			case eStorageType::NVMe: return "NVMe";
			case eStorageType::Removable: return "Removable";
			case eStorageType::CDROM: return "CDROM";
			case eStorageType::RAMDisk: return "RAMDisk";
			case eStorageType::Network: return "Network";
			default: return "Unknown";
			}
		}

		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_Name)
				.and_then([&]() { return s.Serialize(buffer, m_MountPath); })
				.and_then([&]() { return s.Serialize(buffer, m_TotalSizeBytes); })
				.and_then([&]() { return s.Serialize(buffer, m_FreeSizeBytes); })
				.and_then([&]() { return s.Serialize(buffer, m_IsSystemDrive); })
				.and_then([&]() { return s.Serialize(buffer, static_cast<zU8>(m_Type)); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			zU8 rawType = 0;
			return s.Deserialize(buffer, offset, m_Name)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_MountPath); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_FreeSizeBytes); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_TotalSizeBytes); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_IsSystemDrive); })
				.and_then([&]() { return s.Deserialize(buffer, offset, rawType); })
				.and_then([&]() { m_Type = static_cast<eStorageType>(rawType); return std::expected<void, std::string>{}; });
		}

		std::string m_Name;
		std::string m_MountPath;
		zU64 m_TotalSizeBytes{ 0 };
		zU64 m_FreeSizeBytes{ 0 };
		bool m_IsSystemDrive{ false };
		eStorageType m_Type{ eStorageType::Unknown };
	};
}
