#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <filesystem>
#include <core/Enums/ePackage.h>
#include <core/IO/GamePackage/PackageHeader.h>
#include <core/IO/GamePackage/PackageEntry.h>

#include "../core/IO/Path.h"

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

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::optional<T> LoadAssetDataByName(ePackage type, std::string_view name) const
		{
			auto entryOpt = GetEntryByName(type, name);
			if (!entryOpt) return std::nullopt;
			return LoadAssetData<T>(*entryOpt);
		}
		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::optional<T> LoadAssetDataByGuid(ePackage type, const Guid& guid) const
		{
			auto entryOpt = GetEntryByGuid(type, guid);
			if (!entryOpt) return std::nullopt;
			return LoadAssetData<T>(*entryOpt);
		}

	private:
		void Initialize(const Path& path);
		void LogPackageEntriesSummary() const;

		[[nodiscard]] std::optional<PackageEntry> GetEntryByName(ePackage type, std::string_view name) const;
		[[nodiscard]] std::optional<PackageEntry> GetEntryByGuid(ePackage type, const Guid& guid) const;
		template <typename T> requires std::derived_from<T, ISerializable>
		void LogEntriesSummaryForType(ePackage type) const;
		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::optional<T> LoadAssetData(const PackageEntry& entry) const;

		std::filesystem::path m_PackagePath;
		std::map<ePackage, std::unordered_map<std::string, PackageEntry>> m_EntriesByName;
		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
	};
}
