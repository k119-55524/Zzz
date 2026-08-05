#pragma once

#include <map>
#include <string>
#include <optional>
#include <expected>
#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <engine/EngineIncludes.h>
#include <core/IO/package/AppViewData.h>
#include <core/IO/package/PackageEntry.h>

using namespace zzz::core;

namespace zzz::engine
{
	class PackageManager final
	{
	public:
		PackageManager() = delete;
		PackageManager(const Path& path);
		~PackageManager() = default;

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadAssetDataByName(ePackage type, std::string_view name) const
		{
			auto entryOpt = GetEntryByName(type, name);
			if (!entryOpt) return UNEXPECTED("Ассет типа {} с именем '{}' не найден в пакете.", EnumToString::ToString(type), name);
			return LoadAssetData<T>(*entryOpt);
		}
		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadAssetDataByGuid(ePackage type, const Guid& guid) const
		{
			auto entryOpt = GetEntryByGuid(type, guid);
			if (!entryOpt) return UNEXPECTED("Ассет типа {} с GUID '{}' не найден в пакете.", EnumToString::ToString(type), guid.ToString());
			return LoadAssetData<T>(*entryOpt);
		}

		[[nodiscard]] std::optional<PackageEntry> GetEntryByName(ePackage type, std::string_view name) const;
		[[nodiscard]] std::optional<PackageEntry> GetEntryByGuid(ePackage type, const Guid& guid) const;

		[[nodiscard]] std::expected<AppViewData, std::string> GetAppViewData() const;

	private:
		void Initialize(const Path& path);
		void LogPackageEntriesSummary() const;
		template <typename T> requires std::derived_from<T, ISerializable>
		void LogEntriesSummaryForType(ePackage type) const;
		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> LoadAssetData(const PackageEntry& entry) const;

		std::filesystem::path m_PackagePath;
		std::map<ePackage, std::unordered_map<std::string, PackageEntry>> m_EntriesByName;
		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
	};
}
