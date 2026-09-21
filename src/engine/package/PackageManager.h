#pragma once

#include "core/io/FileSystem.h"
#include "engine/EngineIncludes.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/package/PrimaryViewData.h"
#include "core/io/package/ProjectManifestData.h"

using namespace zzz::core;

namespace zzz::core
{
	class SceneData;
	class ChildViewData;
	class IndependentViewData;
	class PrefabData;

	/**
	 * @brief Соответствие между типом ресурса, поддерживаемым архивом package.dat, и его ePackage.
	 * @details Задаёт единственно верный ePackage для каждого T, чтобы вызывающий код
	 *          не мог передать в LoadAsset<T> несовместимый друг с другом тип и ePackage.
	 *          Специализирован только для допустимых типов (ProjectManifestData, PrimaryViewData,
	 *          SceneData, ChildViewData, IndependentViewData, PrefabData) - для любого другого T
	 *          обращение к PackageAssetType<T>::value не скомпилируется (incomplete type).
	 */
	template <typename T>
	struct PackageAssetType;

	template <> struct PackageAssetType<ProjectManifestData> { static constexpr ePackage value = ePackage::ProjectManifest; };
	template <> struct PackageAssetType<PrimaryViewData>     { static constexpr ePackage value = ePackage::PrimaryView; };
	template <> struct PackageAssetType<SceneData>           { static constexpr ePackage value = ePackage::Scene; };
	template <> struct PackageAssetType<ChildViewData>       { static constexpr ePackage value = ePackage::ChildView; };
	template <> struct PackageAssetType<IndependentViewData> { static constexpr ePackage value = ePackage::IndependentView; };
	template <> struct PackageAssetType<PrefabData>          { static constexpr ePackage value = ePackage::Prefab; };

	template <typename T>
	inline constexpr ePackage c_PackageAssetType = PackageAssetType<T>::value;
}

namespace zzz::engine
{
	class SceneManager;

	class PackageManager final
	{
		friend class SceneManager;

	public:
		PackageManager() = delete;
		explicit PackageManager(std::shared_ptr<FileSystem> fileSystem);
		~PackageManager() = default;

		[[nodiscard]] const ProjectManifestData& GetProjectManifestData() const noexcept { return m_ProjectManifest; }
		[[nodiscard]] std::expected<PrimaryViewData, std::string> GetPrimaryViewData() const;

		[[nodiscard]] const DatFileHeader& GetHeader() const noexcept { return m_Header; }
		[[nodiscard]] zU64 GetBuildTime() const noexcept { return m_Header.GetBuildTime(); }

		/// @brief Имя компании и приложения, закэшированные из ProjectManifestData во время Initialize().
		[[nodiscard]] const std::string& GetCompanyName() const noexcept { return m_CompanyName; }
		[[nodiscard]] const std::string& GetAppName() const noexcept { return m_AppName; }

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(std::string_view name) const
		{
			constexpr ePackage type = c_PackageAssetType<T>;
			auto entryOpt = GetEntry(type, name);
			if (!entryOpt)
				return UNEXPECTED("Package entry of type {} with name '{}' was not found.", ToString(type), name);

			return DeserializeEntry<T>(*entryOpt);
		}

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(const Guid& guid) const
		{
			constexpr ePackage type = c_PackageAssetType<T>;
			auto entryOpt = GetEntry(type, guid);
			if (!entryOpt)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString());

			return DeserializeEntry<T>(*entryOpt);
		}

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadRawBytes(const PackageEntry& entry) const;

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] static std::expected<T, std::string> DeserializeEntryFromMemory(const PackageEntry& entry, std::span<const std::byte> bytes);

	private:
		[[nodiscard]] std::optional<PackageEntry> GetEntry(const Guid& guid) const;
		[[nodiscard]] std::optional<PackageEntry> GetEntry(ePackage type, const Guid& guid) const;
		[[nodiscard]] std::optional<PackageEntry> GetEntry(ePackage type, std::string_view name) const;
		void Initialize();

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> DeserializeEntry(const PackageEntry& entry) const;

		void LogPackageEntriesSummary() const;
		template <typename T> requires std::derived_from<T, ISerializable>
		void LogEntriesSummaryForType(ePackage type) const;

		std::shared_ptr<FileSystem> m_FileSystem;
		std::map<ePackage, std::unordered_map<std::string, PackageEntry>> m_EntriesByName;
		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
		std::string m_CompanyName;
		std::string m_AppName;
		ProjectManifestData m_ProjectManifest{};
		DatFileHeader m_Header{};
	};
}
