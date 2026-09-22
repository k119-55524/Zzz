#pragma once

#include <string>
#include <string_view>
#include <span>
#include <vector>

#include "core/utils/Guid.h"
#include "core/logger/logger.h"
#include "core/Serialize/Serializer.h"
#include "core/constants/PackageConstants.h"
#include "core/enums/eFileLocation.h"

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
			zU64 size,
			zU32 pakIndex = 0,
			eFileLocation location = eFileLocation::App)
			: guid(guid)
			, assetType(assetType)
			, offset(offset)
			, size(size)
			, pakIndex(pakIndex)
			, location(location)
		{}

		[[nodiscard]] const Guid& GetGuid() const noexcept { return guid; }
		[[nodiscard]] zU32 GetAssetType() const noexcept { return assetType; }
		[[nodiscard]] zU64 GetOffset() const noexcept { return offset; }
		[[nodiscard]] zU64 GetSize() const noexcept { return size; }
		[[nodiscard]] zU32 GetPakIndex() const noexcept { return pakIndex; }
		[[nodiscard]] eFileLocation GetFileLocation() const noexcept { return location; }

		[[nodiscard]] static constexpr std::size_t BinarySize() noexcept
		{
			// Guid (16) + assetType (4) + offset (8) + size (8) + pakIndex (4) + location (sizeof(eFileLocation))
			return Guid::BinarySize() + sizeof(zU32) + sizeof(zU64) + sizeof(zU64) + sizeof(zU32) + sizeof(eFileLocation);
		}

		inline void LogFileBlock([[maybe_unused]] std::string_view indentation = {}) const
		{
			DOut(Assets, "{}[PackageEntry] guid: {}, type: {}, pak: {}, loc: {}, offset: {}, size: {}",
				indentation,
				guid.ToString(),
				assetType,
				pakIndex,
				ToString(location),
				offset,
				size);
		}

	private:
		Guid guid{};
		zU32 assetType = 0;
		zU64 offset = 0;
		zU64 size = 0;
		zU32 pakIndex = 0;
		eFileLocation location = eFileLocation::App;

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			return serializer.Serialize(buffer, guid)
				.and_then([&]() { return serializer.Serialize(buffer, assetType); })
				.and_then([&]() { return serializer.Serialize(buffer, offset); })
				.and_then([&]() { return serializer.Serialize(buffer, size); })
				.and_then([&]() { return serializer.Serialize(buffer, pakIndex); })
				.and_then([&]() { return serializer.Serialize(buffer, location); });
		}
		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset_, const Serializer& serializer) override
		{
			return serializer.Deserialize(buffer, offset_, guid)
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, assetType); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, offset); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, size); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, pakIndex); })
				.and_then([&]() { return serializer.Deserialize(buffer, offset_, location); });
		}
	};
}
