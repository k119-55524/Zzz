#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <filesystem>
#include <common/enums/ePackage.h>
#include <common/io/package/PackageHeader.h>
#include <common/io/package/PackageEntry.h>

#include "../core/io/Path.h"

using namespace zzz::io;
using namespace zzz::core;
using namespace zzz::common;

namespace zzz::engine
{
	class PackageManager final
	{
	public:
		PackageManager() = delete;
		PackageManager(const Path& path);
		~PackageManager() = default;

		// --- Поиск ресурса человеком (по типу и имени) ---
		[[nodiscard]] std::optional<PackageEntry> GetEntryByName(ePackage type, std::string_view name) const;
		[[nodiscard]] std::optional<PackageEntry> GetSceneByName(std::string_view name) const { return GetEntryByName(ePackage::Scene, name); }
		[[nodiscard]] std::optional<PackageEntry> GetViewByName(std::string_view name) const { return GetEntryByName(ePackage::View, name); }
		[[nodiscard]] std::optional<PackageEntry> GetPrefabByName(std::string_view name) const { return GetEntryByName(ePackage::Prefab, name); }

		// --- Поиск ресурса внутренними системами движка (по типу и GUID) ---
		[[nodiscard]] std::optional<PackageEntry> GetEntryByGuid(ePackage type, const Guid& guid) const;
		[[nodiscard]] std::optional<PackageEntry> GetSceneByGuid(const Guid& guid) const { return GetEntryByGuid(ePackage::Scene, guid); }
		[[nodiscard]] std::optional<PackageEntry> GetViewByGuid(const Guid& guid) const { return GetEntryByGuid(ePackage::View, guid); }
		[[nodiscard]] std::optional<PackageEntry> GetPrefabByGuid(const Guid& guid) const { return GetEntryByGuid(ePackage::Prefab, guid); }

		// --- Таблицы индексов ---
		[[nodiscard]] const std::map<ePackage, std::unordered_map<std::string, PackageEntry>>& GetEntriesByName() const noexcept { return m_EntriesByName; }
		[[nodiscard]] const std::map<ePackage, std::unordered_map<Guid, PackageEntry>>& GetEntriesByGuid() const noexcept { return m_EntriesByGuid; }

	private:
		void Initialize(const Path& path);
		void LogPackageEntriesSummary(const std::filesystem::path& packagePath) const;

		std::map<ePackage, std::unordered_map<std::string, PackageEntry>> m_EntriesByName;
		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
		void LogAssetDetails(const std::filesystem::path& packagePath, const PackageEntry& entry) const;
#endif // Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD
	};
}
