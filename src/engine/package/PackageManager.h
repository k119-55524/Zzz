#pragma once

#include "engine/EngineIncludes.h"
#include "core/io/FileSystem.h"
#include "core/io/package/ProjectManifestData.h"
#include "core/io/package/PrimaryViewData.h"
#include "core/io/package/PackageEntry.h"

using namespace zzz::core;

namespace zzz::engine
{
	class PackageManager final
	{
	public:
		PackageManager() = delete;
		explicit PackageManager(std::shared_ptr<FileSystem> fileSystem);
		~PackageManager() = default;

		[[nodiscard]] std::expected<ProjectManifestData, std::string> GetProjectManifestData() const;
		[[nodiscard]] std::expected<PrimaryViewData, std::string> GetPrimaryViewData() const;

		/// @brief Имя компании и приложения, закэшированные из ProjectManifestData во время Initialize().
		[[nodiscard]] const std::string& GetCompanyName() const noexcept { return m_CompanyName; }
		[[nodiscard]] const std::string& GetAppName() const noexcept { return m_AppName; }

		/// @brief Метаданные (имя/guid/offset) записи Scene в package.dat - для SceneManager
		[[nodiscard]] std::optional<PackageEntry> GetSceneEntryByGuid(const Guid& guid) const { return GetEntryByGuid(ePackage::Scene, guid); }
		[[nodiscard]] std::optional<PackageEntry> GetSceneEntryByName(std::string_view name) const { return GetEntryByName(ePackage::Scene, name); }

		/// @brief Проверяет, есть ли в package.dat запись данного типа с таким guid, без полной загрузки/десериализации.
		[[nodiscard]] bool HasEntry(ePackage type, const Guid& guid) const { return GetEntryByGuid(type, guid).has_value(); }

		/// @brief Возвращает все guid'ы записей данного типа в package.dat
		[[nodiscard]] std::vector<Guid> GetAllGuidsOfType(ePackage type) const
		{
			std::vector<Guid> guids;
			auto it = m_EntriesByGuid.find(type);
			if (it == m_EntriesByGuid.end())
				return guids;

			guids.reserve(it->second.size());
			for (const auto& [guid, entry] : it->second)
				guids.push_back(guid);

			return guids;
		}

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadPackageDataByName(ePackage type, std::string_view name) const
		{
			auto entryOpt = GetEntryByName(type, name);
			if (!entryOpt)
				return UNEXPECTED("Package entry of type {} with name '{}' was not found.", ToString(type), name);

			return LoadPackageData<T>(*entryOpt);
		}
		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadPackageDataByGuid(ePackage type, const Guid& guid) const
		{
			auto entryOpt = GetEntryByGuid(type, guid);
			if (!entryOpt)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString());

			return LoadPackageData<T>(*entryOpt);
		}

	private:
		void Initialize();
		[[nodiscard]] std::optional<PackageEntry> GetEntryByName(ePackage type, std::string_view name) const;
		[[nodiscard]] std::optional<PackageEntry> GetEntryByGuid(ePackage type, const Guid& guid) const;

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadPackageData(const PackageEntry& entry) const;

		void LogPackageEntriesSummary() const;
		template <typename T> requires std::derived_from<T, ISerializable>
		void LogEntriesSummaryForType(ePackage type) const;

		std::shared_ptr<FileSystem> m_FileSystem;
		std::map<ePackage, std::unordered_map<std::string, PackageEntry>> m_EntriesByName;
		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
		std::string m_CompanyName;
		std::string m_AppName;
	};
}
