#pragma once

#include <string>
#include <memory>
#include <optional>
#include <expected>
#include <map>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/Export.h"
#include "core/utils/Ensure.h"
#include "core/io/FileSystem.h"
#include "core/enums/eResourceType.h"
#include "core/serialize/Serializer.h"
#include "core/io/package/PackageEntry.h"

namespace zzz::core
{
	class MeshData;
	class PrefabData;
	class MaterialData;
	class ShaderData;
	class AnimationData;
	class BinaryData;

	/**
	 * @brief Соответствие между типом ресурса, поддерживаемым архивом data.dat, и его eResourceType.
	 * @details Задаёт единственно верный eResourceType для каждого T, чтобы вызывающий код
	 *          не мог передать в LoadAsset<T> несовместимый друг с другом тип и eResourceType.
	 *          Специализирован только для допустимых типов (MeshData, PrefabData, MaterialData,
	 *          ShaderData, AnimationData, BinaryData) - для любого другого T обращение
	 *          к DataAssetResourceType<T>::value не скомпилируется (incomplete type).
	 */
	template <typename T>
	struct DataAssetResourceType;

	template <> struct DataAssetResourceType<MeshData>      { static constexpr eResourceType value = eResourceType::Mesh; };
	template <> struct DataAssetResourceType<PrefabData>    { static constexpr eResourceType value = eResourceType::Prefab; };
	template <> struct DataAssetResourceType<MaterialData>  { static constexpr eResourceType value = eResourceType::Material; };
	template <> struct DataAssetResourceType<ShaderData>    { static constexpr eResourceType value = eResourceType::Shader; };
	template <> struct DataAssetResourceType<AnimationData> { static constexpr eResourceType value = eResourceType::Animation; };
	template <> struct DataAssetResourceType<BinaryData>    { static constexpr eResourceType value = eResourceType::BinaryData; };

	template <typename T>
	inline constexpr eResourceType c_DataAssetResourceType = DataAssetResourceType<T>::value;

	/**
	 * @class DataAssetsManager
	 * @brief Менеджер для чтения игровых ресурсов из архива assets/data/data.dat (меши, материалы, шейдеры, префабы).
	 */
	class Z_CORE_API DataAssetsManager final
	{
	public:
		DataAssetsManager() = delete;
		explicit DataAssetsManager(std::shared_ptr<FileSystem> fileSystem);
		~DataAssetsManager() = default;

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(const Guid& guid) const
		{
			constexpr eResourceType expectedType = c_DataAssetResourceType<T>;
			auto entryOpt = GetEntry(expectedType, guid);
			if (!entryOpt)
				return UNEXPECTED("Ресурс типа {} с GUID '{}' не найден в data.dat", ToString(expectedType), guid.ToString());

			return DeserializeEntry<T>(*entryOpt);
		}

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(std::string_view name) const
		{
			constexpr eResourceType expectedType = c_DataAssetResourceType<T>;
			auto entryOpt = GetEntry(expectedType, name);
			if (!entryOpt)
				return UNEXPECTED("Ресурс типа {} с именем '{}' не найден в data.dat", ToString(expectedType), name);

			return DeserializeEntry<T>(*entryOpt);
		}

	private:
		[[nodiscard]] std::optional<PackageEntry> GetEntry(eResourceType type, const Guid& guid) const;
		[[nodiscard]] std::optional<PackageEntry> GetEntry(eResourceType type, std::string_view name) const;

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> DeserializeEntry(const PackageEntry& entry) const
		{
			auto bufferRes = m_FileSystem->ReadBytes(eFileLocation::App, c_DataPackageRelativePath, entry.GetOffset(), entry.GetSize());
			if (!bufferRes)
				return UNEXPECTED("Не удалось прочитать блок данных '{}' из пакета '{}': {}",
					entry.GetName(), c_DataPackageRelativePath, bufferRes.error());

			const auto& buffer = *bufferRes;
			std::size_t offset = 0;
			Serializer serializer;
			T data{};
			auto res = serializer.Deserialize(buffer, offset, data);
			if (!res)
				return UNEXPECTED("Ошибка десериализации ресурса '{}': {}", entry.GetName(), res.error());

			return data;
		}

		void Initialize();
		void LogDataEntriesSummary() const;

		std::shared_ptr<FileSystem> m_FileSystem;
		std::map<eResourceType, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
		std::map<eResourceType, std::unordered_map<std::string, PackageEntry>> m_EntriesByName;
	};
}
