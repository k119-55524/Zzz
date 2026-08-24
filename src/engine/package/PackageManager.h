#pragma once

#include "engine/EngineIncludes.h"

using namespace zzz::core;

namespace zzz::engine
{
	class PackageManager final
	{
	public:
		PackageManager() = delete;
		PackageManager(const Path& path);
		~PackageManager() = default;

		[[nodiscard]] std::expected<ProjectManifestData, std::string> GetProjectManifestData() const;
		[[nodiscard]] std::expected<PrimaryViewData, std::string> GetPrimaryViewData() const;

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadPackageDataByName(ePackage type, std::string_view name) const
		{
			auto entryOpt = GetEntryByName(type, name);
			if (!entryOpt)
				return UNEXPECTED("Package entry of type {} with name '{}' was not found.", EnumToString::ToString(type), name);

			return LoadPackageData<T>(*entryOpt);
		}
		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadPackageDataByGuid(ePackage type, const Guid& guid) const
		{
			auto entryOpt = GetEntryByGuid(type, guid);
			if (!entryOpt)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", EnumToString::ToString(type), guid.ToString());

			return LoadPackageData<T>(*entryOpt);
		}

	private:
		void Initialize(const Path& path);
		[[nodiscard]] std::optional<PackageEntry> GetEntryByName(ePackage type, std::string_view name) const;
		[[nodiscard]] std::optional<PackageEntry> GetEntryByGuid(ePackage type, const Guid& guid) const;

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadPackageData(const PackageEntry& entry) const;

		void LogPackageEntriesSummary() const;
		template <typename T> requires std::derived_from<T, ISerializable>
		void LogEntriesSummaryForType(ePackage type) const;

		std::filesystem::path m_PackagePath;
		std::map<ePackage, std::unordered_map<std::string, PackageEntry>> m_EntriesByName;
		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
	};
}
