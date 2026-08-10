#pragma once

#include "core/CoreIncludes.h"
#include <logger/logger.h>
#include "core/enums/eEnumToString.h"
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	enum class eGPUType : zU8
	{
		Unknown = 0,
		Discrete,
		Integrated,
		CpuSoftware
	};

	class GpuInfo final : public ISerializable
	{
	public:
		GpuInfo() = default;
		GpuInfo(std::string platformGpuId, std::string name, zU32 vendorId, zU32 deviceId, eGPUType type,
			zU64 dedicatedVideoMemoryBytes, zU64 sharedSystemMemoryBytes, zU64 dedicatedSystemMemoryBytes,
			zU64 score = 0, bool isCanDisableVSync = false)
			: m_PlatformGpuId(std::move(platformGpuId))
			, m_Name(std::move(name))
			, m_VendorId(vendorId)
			, m_DeviceId(deviceId)
			, m_Type(type)
			, m_DedicatedVideoMemoryBytes(dedicatedVideoMemoryBytes)
			, m_SharedSystemMemoryBytes(sharedSystemMemoryBytes)
			, m_DedicatedSystemMemoryBytes(dedicatedSystemMemoryBytes)
			, m_Score(score)
			, m_IsCanDisableVSync(isCanDisableVSync)
		{}

		[[nodiscard]] const std::string& GetPlatformGpuId() const noexcept { return m_PlatformGpuId; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		[[nodiscard]] zU32 GetVendorId() const noexcept { return m_VendorId; }
		[[nodiscard]] zU32 GetDeviceId() const noexcept { return m_DeviceId; }
		[[nodiscard]] eGPUType GetType() const noexcept { return m_Type; }
		[[nodiscard]] zU64 GetDedicatedVideoMemoryBytes() const noexcept { return m_DedicatedVideoMemoryBytes; }
		[[nodiscard]] zU64 GetSharedSystemMemoryBytes() const noexcept { return m_SharedSystemMemoryBytes; }
		[[nodiscard]] zU64 GetDedicatedSystemMemoryBytes() const noexcept { return m_DedicatedSystemMemoryBytes; }
		[[nodiscard]] zU64 GetScore() const noexcept { return m_Score; }
		[[nodiscard]] bool IsCanDisableVSync() const noexcept { return m_IsCanDisableVSync; }

		[[nodiscard]] bool operator==(const GpuInfo& other) const noexcept
		{
			return m_PlatformGpuId == other.m_PlatformGpuId &&
				m_Name == other.m_Name &&
				m_VendorId == other.m_VendorId &&
				m_DeviceId == other.m_DeviceId &&
				m_Type == other.m_Type &&
				m_DedicatedVideoMemoryBytes == other.m_DedicatedVideoMemoryBytes;
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			const std::string nestedIndentation = std::string(indentation) + "  ";
			DOut("{}[GpuInfo]", indentation);
			DOut("{}platformGpuId: {}", nestedIndentation, m_PlatformGpuId);
			DOut("{}name: {}", nestedIndentation, m_Name);
			DOut("{}vendorId: 0x{:04X}", nestedIndentation, m_VendorId);
			DOut("{}deviceId: 0x{:04X}", nestedIndentation, m_DeviceId);
			DOut("{}type: {}", nestedIndentation, static_cast<zU32>(m_Type));
			DOut("{}dedicatedVideoMemoryBytes: {} MB", nestedIndentation, m_DedicatedVideoMemoryBytes / (1024 * 1024));
			DOut("{}sharedSystemMemoryBytes: {} MB", nestedIndentation, m_SharedSystemMemoryBytes / (1024 * 1024));
			DOut("{}dedicatedSystemMemoryBytes: {} MB", nestedIndentation, m_DedicatedSystemMemoryBytes / (1024 * 1024));
			DOut("{}score: {}", nestedIndentation, m_Score);
			DOut("{}isCanDisableVSync: {}", nestedIndentation, m_IsCanDisableVSync);
#endif
		}

	private:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& s) const override
		{
			return s.Serialize(buffer, m_PlatformGpuId)
				.and_then([&]() { return s.Serialize(buffer, m_Name); })
				.and_then([&]() { return s.Serialize(buffer, m_VendorId); })
				.and_then([&]() { return s.Serialize(buffer, m_DeviceId); })
				.and_then([&]() { return s.Serialize(buffer, static_cast<zU8>(m_Type)); })
				.and_then([&]() { return s.Serialize(buffer, m_DedicatedVideoMemoryBytes); })
				.and_then([&]() { return s.Serialize(buffer, m_SharedSystemMemoryBytes); })
				.and_then([&]() { return s.Serialize(buffer, m_DedicatedSystemMemoryBytes); })
				.and_then([&]() { return s.Serialize(buffer, m_Score); })
				.and_then([&]() { return s.Serialize(buffer, m_IsCanDisableVSync); });
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset, const Serializer& s) override
		{
			zU8 rawType = 0;
			return s.Deserialize(buffer, offset, m_PlatformGpuId)
				.and_then([&]() { return s.Deserialize(buffer, offset, m_Name); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_VendorId); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_DeviceId); })
				.and_then([&]() { return s.Deserialize(buffer, offset, rawType); })
				.and_then([&]() -> std::expected<void, std::string> {
					m_Type = static_cast<eGPUType>(rawType);
					return {};
				})
				.and_then([&]() { return s.Deserialize(buffer, offset, m_DedicatedVideoMemoryBytes); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_SharedSystemMemoryBytes); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_DedicatedSystemMemoryBytes); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_Score); })
				.and_then([&]() { return s.Deserialize(buffer, offset, m_IsCanDisableVSync); });
		}

		std::string m_PlatformGpuId;
		std::string m_Name;
		zU32 m_VendorId{ 0 };
		zU32 m_DeviceId{ 0 };
		eGPUType m_Type{ eGPUType::Unknown };

		zU64 m_DedicatedVideoMemoryBytes{ 0 };
		zU64 m_SharedSystemMemoryBytes{ 0 };
		zU64 m_DedicatedSystemMemoryBytes{ 0 };

		zU64 m_Score{ 0 };
		bool m_IsCanDisableVSync{ false };
	};
}
