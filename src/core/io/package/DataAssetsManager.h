#pragma once

#include <memory>
#include <unordered_map>
#include <optional>
#include <expected>
#include <string>
#include <concepts>
#include "core/utils/Export.h"
#include "core/utils/Guid.h"
#include "core/utils/Ensure.h"
#include "core/io/FileSystem.h"
#include "core/io/package/PackageEntry.h"
#include "core/enums/eResourceType.h"
#include "core/serialize/Serializer.h"

namespace zzz::core
{
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

		[[nodiscard]] bool HasEntry(const Guid& guid) const noexcept
		{
			return m_EntriesByGuid.contains(guid);
		}

		[[nodiscard]] std::optional<PackageEntry> GetEntryByGuid(const Guid& guid) const
		{
			auto it = m_EntriesByGuid.find(guid);
			if (it == m_EntriesByGuid.end())
				return std::nullopt;
			return it->second;
		}

		[[nodiscard]] std::optional<PackageEntry> GetEntryByName(std::string_view name) const
		{
			auto it = m_EntriesByName.find(std::string(name));
			if (it == m_EntriesByName.end())
				return std::nullopt;
			return it->second;
		}

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadData(eResourceType expectedType, const Guid& guid) const
		{
			ensure(
				expectedType == eResourceType::Mesh ||
				expectedType == eResourceType::Prefab ||
				expectedType == eResourceType::Material ||
				expectedType == eResourceType::Shader ||
				expectedType == eResourceType::Animation ||
				expectedType == eResourceType::BinaryData,
				"Запрошенный тип ресурса не поддерживается архивом data.dat");

			auto it = m_EntriesByGuid.find(guid);
			if (it == m_EntriesByGuid.end())
				return UNEXPECTED("Ресурс с GUID '{}' не найден в data.dat", guid.ToString());

			const auto& entry = it->second;
			ensure(static_cast<eResourceType>(entry.GetAssetType()) == expectedType,
				"Несовпадение типа ресурса: ожидался {}, фактически {}",
				ToString(expectedType), ToString(static_cast<eResourceType>(entry.GetAssetType())));

			return LoadData<T>(entry);
		}

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadData(const PackageEntry& entry) const
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

	private:
		void Initialize();
		void LogDataEntriesSummary() const;

		std::shared_ptr<FileSystem> m_FileSystem;
		std::unordered_map<Guid, PackageEntry> m_EntriesByGuid;
		std::unordered_map<std::string, PackageEntry> m_EntriesByName;
	};
}
