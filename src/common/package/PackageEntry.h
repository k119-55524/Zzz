#pragma once

#include <common/guid.h>
#include <common/serialize/Serializer.h>

using namespace zzz::common;

namespace zzz::core
{
	class PackageEntry final : public ISerializable
	{
	public:
		PackageEntry() = default;
		PackageEntry(const Guid& guid, zU32 assetType, zU64 offset, zU64 size)
			: guid(guid)
			, assetType(assetType)
			, offset(offset)
			, size(size)
		{}

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
