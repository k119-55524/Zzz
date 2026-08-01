#pragma once

#include <string>
#include <core/Guid.h>
#include <core/serialize/Serializer.h>

using namespace zzz::io;
using namespace zzz::common;

namespace zzz::core
{
	class PackageEntry final : public ISerializable
	{
	public:
		PackageEntry() = default;
		PackageEntry(std::string name, const Guid& guid, zU32 assetType, zU64 offset, zU64 size)
			: name(std::move(name))
			, guid(guid)
			, assetType(assetType)
			, offset(offset)
			, size(size)
		{}

		std::string name;
		Guid guid{};
		zU32 assetType = 0;
		zU64 offset = 0;
		zU64 size = 0;

		void LogFileBlock() const
		{
#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
			DOut("        [PackageEntry] Имя: '{}' | GUID: {} | Тип: {} | Смещение: {} байт | Размер: {} байт",
				name, guid.ToString(), assetType, offset, size);
#endif
		}

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
