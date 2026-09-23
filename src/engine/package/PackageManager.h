#pragma once

#include "core/io/FileSystem.h"
#include "core/enums/ePackage.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/package/PrimaryViewData.h"
#include "core/io/package/ProjectManifestData.h"

namespace zzz::core
{
	class SceneData;
	class PrefabData;
	class ChildViewData;
	class IndependentViewData;
}

namespace zzz::engine
{
	class SceneManager;

	class PackageManager final
	{
	public:
		PackageManager() = delete;
		explicit PackageManager(std::shared_ptr<FileSystem> fileSystem);
		~PackageManager() = default;

		[[nodiscard]] const ProjectManifestData& GetProjectManifestData() const noexcept { return m_ProjectManifest; }
		[[nodiscard]] std::expected<PrimaryViewData, std::string> GetPrimaryViewData() const;

		/// @brief Имя компании и приложения, закэшированные из ProjectManifestData во время Initialize().
		[[nodiscard]] const std::string& GetCompanyName() const noexcept { return m_CompanyName; }
		[[nodiscard]] const std::string& GetAppName() const noexcept { return m_AppName; }

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(const Guid& guid) const
		{
			constexpr ePackage type = T::c_PackageType;
			auto entryOpt = GetEntry(type, guid);
			if (!entryOpt)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString());

			return DeserializeEntry<T>(*entryOpt);
		}

		[[nodiscard]] std::optional<Guid> FindSceneGuidByName(std::string_view name) const noexcept;

	private:
		[[nodiscard]] std::optional<PackageEntry> GetEntry(ePackage type, const Guid& guid) const;
		void Initialize();

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadRawBytes(const PackageEntry& entry) const;

		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> DeserializeEntry(const PackageEntry& entry) const;

		void LogPackageEntriesSummary() const;
		template <typename T> requires std::derived_from<T, ISerializable>
		void LogEntriesSummaryForType(ePackage type) const;

		struct StringHash
		{
			using is_transparent = void;
			[[nodiscard]] size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
		};

		std::shared_ptr<FileSystem> m_FileSystem;
		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;
		std::unordered_map<std::string, Guid, StringHash, std::equal_to<>> m_SceneGuidsByName;
		std::string m_CompanyName;
		std::string m_AppName;
		ProjectManifestData m_ProjectManifest{};
		DatFileHeader m_Header{};
	};
}
