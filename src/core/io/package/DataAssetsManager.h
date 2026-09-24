#pragma once

#include <span>
#include <string>
#include <memory>
#include <expected>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/io/FileSystem.h"
#include "core/enums/eResourceType.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/package/PackageArchive.h"

namespace zzz::core
{
	/**
	 * @class DataAssetsManager
	 * @brief Менеджер для чтения игровых ресурсов из архива data.dat.
	 */
	class Z_CORE_API DataAssetsManager final : public PackageArchive<eResourceType>
	{
	public:
		DataAssetsManager() = delete;
		explicit DataAssetsManager(std::shared_ptr<FileSystem> fileSystem);
		~DataAssetsManager() override = default;

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(const Guid& guid) const
		{
			constexpr eResourceType type = T::c_ResourceType;
			const auto* entry = GetEntry(type, guid);
			if (!entry)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString());

			auto payloadRes = ReadRawPayload(*entry);
			if (!payloadRes)
				return UNEXPECTED("{}", payloadRes.error());

			return DeserializeAssetFromMemory<T>(*entry, payloadRes->GetSpan());
		}

		template <typename T>
		[[nodiscard]] static std::expected<T, std::string> DeserializeAssetFromMemory(
			const PackageEntry& entry,
			std::span<const std::byte> bytes)
		{
			constexpr eResourceType expectedType = T::c_ResourceType;
			if (entry.GetAssetType() != static_cast<zU32>(expectedType))
				return UNEXPECTED("Несоответствие типа ассета с GUID '{}'. Ожидался: {}, в записи: {}",
					entry.GetGuid().ToString(), ToString(expectedType), entry.GetAssetType());

			std::size_t offset = 0;
			Serializer serializer;
			T data{};
			auto res = serializer.Deserialize(bytes, offset, data);
			if (!res)
				return UNEXPECTED("Ошибка десериализации ресурса с GUID '{}': {}", entry.GetGuid().ToString(), res.error());

			return data;
		}
	};
}
