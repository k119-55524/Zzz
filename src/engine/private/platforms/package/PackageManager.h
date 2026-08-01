#pragma once

#include <map>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <filesystem>
#include <core/enums/ePackage.h>
#include <core/io/gamepackage/PackageHeader.h>
#include <core/io/gamepackage/PackageEntry.h>

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

		// Поиск ресурса по имени
		[[nodiscard]] std::optional<PackageEntry> GetEntryByName(ePackage type, std::string_view name) const;
		[[nodiscard]] std::optional<PackageEntry> GetSceneByName(std::string_view name) const { return GetEntryByName(ePackage::Scene, name); }
		[[nodiscard]] std::optional<PackageEntry> GetViewByName(std::string_view name) const { return GetEntryByName(ePackage::View, name); }
		[[nodiscard]] std::optional<PackageEntry> GetPrefabByName(std::string_view name) const { return GetEntryByName(ePackage::Prefab, name); }

		// Поиск ресурса по GUID
		[[nodiscard]] std::optional<PackageEntry> GetEntryByGuid(ePackage type, const Guid& guid) const;
		[[nodiscard]] std::optional<PackageEntry> GetSceneByGuid(const Guid& guid) const { return GetEntryByGuid(ePackage::Scene, guid); }
		[[nodiscard]] std::optional<PackageEntry> GetViewByGuid(const Guid& guid) const { return GetEntryByGuid(ePackage::View, guid); }
		[[nodiscard]] std::optional<PackageEntry> GetPrefabByGuid(const Guid& guid) const { return GetEntryByGuid(ePackage::Prefab, guid); }

		// --- Загрузка структур данных ассетов ---
		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::optional<T> LoadAssetData(const PackageEntry& entry) const
		{
			if (m_PackagePath.empty()) return std::nullopt;

			std::ifstream file(m_PackagePath, std::ios::binary);
			if (!file.is_open()) return std::nullopt;

			file.seekg(entry.offset, std::ios::beg);
			std::vector<std::byte> buffer(entry.size);
			file.read(reinterpret_cast<char*>(buffer.data()), entry.size);
			if (!file.good()) return std::nullopt;

			std::size_t offset = 0;
			Serializer serializer;
			T data{};
			if (serializer.Deserialize(buffer, offset, data))
			{
				return data;
			}
			return std::nullopt;
		}

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

		template <typename T> requires std::derived_from<T, ISerializable>
		void LogEntriesSummaryForType(ePackage type) const;

		std::filesystem::path m_PackagePath;
		std::map<ePackage, std::unordered_map<std::string, PackageEntry>> m_EntriesByName;
		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
	};
}
