#pragma once

#include <span>
#include <string>
#include <expected>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/enums/eDataDatType.h"
#include "core/enums/eEngineResourceType.h"
#include "core/enums/ResourceDatMapping.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/package/ArchiveReaderBase.h"

namespace zzz::core
{
	/**
	 * @class DataAssetsManager
	 * @brief Менеджер для чтения игровых ресурсов из архива data.dat.
	 */
	class Z_CORE_API DataAssetsManager final : public ArchiveReaderBase<eDataDatType>
	{
	public:
		DataAssetsManager() = delete;
		explicit DataAssetsManager(const std::filesystem::path& physicalPath);
		~DataAssetsManager() override = default;

		using ArchiveReaderBase<eDataDatType>::GetEntry;
		using ArchiveReaderBase<eDataDatType>::HasEntry;

		/// @brief Удобный поиск записи по общему типу ресурса движка.
		[[nodiscard]] const PackageEntry* GetEntry(eEngineResourceType type, const Guid& guid) const noexcept
		{
			const auto dataDatType = TryToDataDatType(type);
			if (!dataDatType)
				return nullptr;
			return ArchiveReaderBase<eDataDatType>::GetEntry(*dataDatType, guid);
		}

		[[nodiscard]] bool HasEntry(eEngineResourceType type, const Guid& guid) const noexcept
		{
			return GetEntry(type, guid) != nullptr;
		}

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(const Guid& guid) const
		{
			static_assert(requires { { T::c_DataDatType } -> std::convertible_to<eDataDatType>; } ||
			              requires { { T::c_ResourceType } -> std::convertible_to<eEngineResourceType>; },
				"T must define static constexpr eDataDatType c_DataDatType or eEngineResourceType c_ResourceType");

			constexpr eDataDatType type = []() constexpr {
				if constexpr (requires { { T::c_DataDatType } -> std::convertible_to<eDataDatType>; })
					return T::c_DataDatType;
				else
					return ToDataDatType(T::c_ResourceType);
			}();

			static_assert(ArchiveTraits<eDataDatType>::IsTypeAllowed(type),
				"Asset type is not allowed in data.dat");

			const auto* entry = GetEntry(type, guid);
			if (!entry)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString());

			auto payloadRes = ReadRawPayload(*entry);
			if (!payloadRes)
				return UNEXPECTED("{}", payloadRes.error());

			return DeserializeAssetFromMemory<T>(*entry, *payloadRes);
		}

		template <typename T>
		[[nodiscard]] static std::expected<T, std::string> DeserializeAssetFromMemory(
			const PackageEntry& entry,
			std::span<const std::byte> bytes)
		{
			constexpr eDataDatType expectedType = []() constexpr {
				if constexpr (requires { { T::c_DataDatType } -> std::convertible_to<eDataDatType>; })
					return T::c_DataDatType;
				else
					return ToDataDatType(T::c_ResourceType);
			}();

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
