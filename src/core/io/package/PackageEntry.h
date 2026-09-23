#pragma once

#include <span>
#include <vector>
#include <string>
#include <string_view>


#include "core/utils/Guid.h"
#include "core/logger/logger.h"
#include "core/Serialize/Serializer.h"
#include "core/constants/PackageConstants.h"

namespace zzz::core
{
	class PackageEntry final : public ISerializable
	{
	public:
		PackageEntry() = default;
		PackageEntry(
			const Guid& guid,
			zU32 assetType,
			zU64 offset,
			zU64 size)
			: guid(guid)
			, assetType(assetType)
			, offset(offset)
			, size(size)
		{}

		[[nodiscard]] const Guid& GetGuid() const noexcept { return guid; }
		[[nodiscard]] zU32 GetAssetType() const noexcept { return assetType; }
		[[nodiscard]] zU64 GetOffset() const noexcept { return offset; }
		[[nodiscard]] zU64 GetSize() const noexcept { return size; }

		[[nodiscard]] static constexpr std::size_t BinarySize() noexcept
		{
			// Guid (16) + assetType (4) + offset (8) + size (8) = 36 байт
			return Guid::BinarySize() + sizeof(zU32) + sizeof(zU64) + sizeof(zU64);
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
			DOut(Assets, "{}[PackageEntry] guid: {}, type: {}, offset: {}, size: {}",
				indentation,
				guid.ToString(),
				assetType,
				offset,
				size);
		}

	private:
		Guid guid{};
		zU32 assetType = 0;
		zU64 offset = 0;
		zU64 size = 0;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, guid)
				.and_then([&]() { return serializer.Serialize(buffer, assetType); })
				.and_then([&]() { return serializer.Serialize(buffer, offset); })
				.and_then([&]() { return serializer.Serialize(buffer, size); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset_, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset_, guid)
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, assetType); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, offset); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, size); });
		}
	};
}
