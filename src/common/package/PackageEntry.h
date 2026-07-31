#pragma once

#include <array>
#include <cstring>
#include <common/guid.h>
#include <common/serialize/Serializer.h>

using namespace zzz::common;

namespace zzz::core
{
	class PackageEntry final : public ISerializable
	{
	public:
		PackageEntry() = default;
		PackageEntry(const std::array<std::byte, 16>& guidBytes, zU32 assetType, zU64 offset, zU64 size)
			: guidBytes(guidBytes)
			, assetType(assetType)
			, offset(offset)
			, size(size)
		{}

		std::array<std::byte, 16> guidBytes = {};
		zU32 assetType = 0;
		zU64 offset = 0;
		zU64 size = 0;

		[[nodiscard]] Guid GetGuid() const noexcept
		{
			Guid::RawBytes raw{};
			std::memcpy(raw.data(), guidBytes.data(), 16);
			return Guid{ raw };
		}

		void SetGuid(const Guid& guid) noexcept
		{
			std::memcpy(guidBytes.data(), guid.GetBytes().data(), 16);
		}

	protected:
		[[nodiscard]] std::expected<void, std::string> Serialize(std::vector<std::byte>& buffer, const Serializer& serializer) const override
		{
			auto res = serializer.Serialize(buffer, guidBytes);
			if (!res) return res;

			res = serializer.Serialize(buffer, assetType);
			if (!res) return res;

			res = serializer.Serialize(buffer, offset);
			if (!res) return res;

			res = serializer.Serialize(buffer, size);
			if (!res) return res;

			return {};
		}

		[[nodiscard]] std::expected<void, std::string> Deserialize(std::span<const std::byte> buffer, std::size_t& offset_, const Serializer& serializer) override
		{
			auto res = serializer.Deserialize(buffer, offset_, guidBytes);
			if (!res) return res;

			res = serializer.Deserialize(buffer, offset_, assetType);
			if (!res) return res;

			res = serializer.Deserialize(buffer, offset_, offset);
			if (!res) return res;

			res = serializer.Deserialize(buffer, offset_, size);
			if (!res) return res;

			return {};
		}
	};
}
