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
		explicit DataAssetsManager(
			const std::filesystem::path& physicalPath,
			NativeAppData* nativeData = nullptr);
		~DataAssetsManager() override = default;

		using ArchiveReaderBase<eDataDatType>::GetEntry;
		using ArchiveReaderBase<eDataDatType>::HasEntry;

		/**
		 * @brief Поиск записи ресурса по общему типу движка.
		 * @param type Общий тип ресурса движка (eEngineResourceType).
		 * @param guid Уникальный идентификатор ресурса.
		 * @return Указатель на PackageEntry или nullptr, если тип не поддерживается data.dat или запись отсутствует.
		 */
		[[nodiscard]] const PackageEntry* GetEntry(eEngineResourceType type, const Guid& guid) const noexcept
		{
			const auto dataDatType = TryToDataDatType(type);
			if (!dataDatType)
				return nullptr;
			return ArchiveReaderBase<eDataDatType>::GetEntry(*dataDatType, guid);
		}

		/**
		 * @brief Проверка наличия записи ресурса по общему типу движка.
		 * @param type Общий тип ресурса движка (eEngineResourceType).
		 * @param guid Уникальный идентификатор ресурса.
		 * @return true, если запись с указанным GUID и типом присутствует в data.dat.
		 */
		[[nodiscard]] bool HasEntry(eEngineResourceType type, const Guid& guid) const noexcept
		{
			return GetEntry(type, guid) != nullptr;
		}

		/**
		 * @brief Поиск записи ресурса по GUID для типизированных структур ассетов (Mesh, Material, Texture и др.).
		 * @tparam T Тип ассета, определяющий static constexpr eDataDatType c_DataDatType или eEngineResourceType c_ResourceType.
		 * @param guid Уникальный идентификатор ресурса.
		 * @return Указатель на PackageEntry или nullptr, если запись не найдена.
		 */
		template <typename T>
		[[nodiscard]] const PackageEntry* GetEntry(const Guid& guid) const noexcept
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

			return ArchiveReaderBase<eDataDatType>::GetEntry<type>(guid);
		}

		/**
		 * @brief Загружает и десериализует ресурс игровых данных заданного типа из архива data.dat.
		 * @tparam T Тип ассета.
		 * @param guid Уникальный идентификатор ресурса.
		 * @return Экземпляр T в случае успеха, либо строка с описанием ошибки.
		 */
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

			ensure(ArchiveTraits<eDataDatType>::IsTypeAllowed(type), "Тип ассета недопустим в data.dat");

			const auto* entry = GetEntry<type>(guid);
			if (!entry)
				return UNEXPECTED("Запись пакета с типом {} и GUID '{}' не найдена.", ToString(type), guid.ToString());

			auto payloadRes = ReadRawPayload(*entry);
			if (!payloadRes)
				return UNEXPECTED("{}", payloadRes.error());

			return DeserializeAssetFromMemory<T>(*entry, *payloadRes);
		}

		/**
		 * @brief Десериализует ассет из переданного буфера сырых байтов с валидацией типа записи.
		 * @tparam T Тип десериализуемой структуры ассета.
		 * @param entry Метаданные записи пакета.
		 * @param bytes Буфер сырых байтов данных ресурса.
		 * @return Десериализованный объект T или ошибка десериализации.
		 */
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
