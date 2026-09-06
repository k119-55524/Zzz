#pragma once

#include <string>
#include <string_view>
#include "core/utils/Guid.h"
#include "core/utils/FixedLengthString.h"
#include "core/constants/PackageConstants.h"
#include <logger/logger.h>
#include "core/Serialize/Serializer.h"

namespace zzz::core
{
	class PackageEntry final : public ISerializable
	{
	public:
		using NameStringType = FixedLengthString32<c_MaxAssetNameLength>;

		PackageEntry() = default;
		PackageEntry(NameStringType name, const Guid& guid, zU32 assetType, zU64 offset, zU64 size)
			: name(name)
			, guid(guid)
			, assetType(assetType)
			, offset(offset)
			, size(size)
		{}

		[[nodiscard]] std::string GetName() const { return name.ToString(); }
		[[nodiscard]] const NameStringType& GetFixedName() const noexcept { return name; }
		[[nodiscard]] const Guid& GetGuid() const noexcept { return guid; }
		[[nodiscard]] zU32 GetAssetType() const noexcept { return assetType; }
		[[nodiscard]] zU64 GetOffset() const noexcept { return offset; }
		[[nodiscard]] zU64 GetSize() const noexcept { return size; }

		[[nodiscard]] static constexpr std::size_t BinarySize() noexcept
		{
			// FixedLengthString32<N> (N * 4 bytes) + Guid (16 bytes) + assetType (4 bytes) + offset (8 bytes) + size (8 bytes)
			return NameStringType::BinarySize() + Guid::BinarySize() + sizeof(zU32) + sizeof(zU64) + sizeof(zU64);
		}

		inline void LogFileBlock(std::string_view indentation = {}) const
		{
			DOut(::zzz::core::Assets, "{}[PackageEntry] name: '{}', guid: {}, type: {}, offset: {}, size: {}",
				indentation,
				GetName(),
				guid.ToString(),
				assetType,
				offset,
				size);
		}

	private:
		NameStringType name{};
		Guid guid{};
		zU32 assetType = 0;
		zU64 offset = 0;
		zU64 size = 0;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, name)
				.and_then([&]() { return serializer.Serialize(buffer, guid); })
				.and_then([&]() { return serializer.Serialize(buffer, assetType); })
				.and_then([&]() { return serializer.Serialize(buffer, offset); })
				.and_then([&]() { return serializer.Serialize(buffer, size); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset_, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset_, name)
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, guid); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, assetType); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, offset); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, size); });
		}
	};
}

