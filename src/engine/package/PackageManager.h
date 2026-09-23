#pragma once

#include "core/io/FileSystem.h"
#include "core/enums/ePackage.h"
#include "core/io/DatFileHeader.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/io/package/PackageEntry.h"
#include "core/constants/PackagesConstants.h"
#include "core/io/package/ProjectManifestData.h"
#include "core/io/package/views/PrimaryViewData.h"

namespace zzz::engine
{
	class PackageManager final
	{
	public:
		explicit PackageManager(std::shared_ptr<FileSystem> fileSystem);
		~PackageManager() = default;

		[[nodiscard]] const DatFileHeader& GetHeader() const noexcept { return m_Header; }
		[[nodiscard]] const ProjectManifestData& GetProjectManifestData() const noexcept { return m_ProjectManifest; }
		[[nodiscard]] std::expected<PrimaryViewData, std::string> GetPrimaryViewData() const;

		/// @brief Имя компании и приложения из ProjectManifestData.
		[[nodiscard]] const std::string& GetCompanyName() const noexcept { return m_ProjectManifest.GetCompanyName(); }
		[[nodiscard]] const std::string& GetAppName() const noexcept { return m_ProjectManifest.GetAppName(); }

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(const Guid& guid) const
		{
			constexpr ePackage type = T::c_PackageType;
			const auto* entry = GetEntry(type, guid);
			if (!entry)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString());

			return DeserializeEntry<T>(*entry);
		}

		[[nodiscard]] std::optional<Guid> FindSceneGuidByName(std::string_view name) const noexcept;

	private:
		void Initialize();

		[[nodiscard]] std::expected<std::vector<std::byte>, std::string> ReadRawBytes(const PackageEntry& entry) const;
		[[nodiscard]] const PackageEntry* GetEntry(ePackage type, const Guid& guid) const noexcept;
		template <typename T> requires std::derived_from<T, ISerializable>
		[[nodiscard]] std::expected<T, std::string> DeserializeEntry(const PackageEntry& entry) const;

		void LogPackageEntriesSummary() const;
		template <typename T> requires std::derived_from<T, ISerializable>
		void LogEntriesSummaryForType(ePackage type) const;


		std::map<ePackage, std::unordered_map<Guid, PackageEntry>> m_EntriesByGuid;

		struct StringHash
		{
			using is_transparent = void;
			[[nodiscard]] size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
		};
		std::unordered_map<std::string, Guid, StringHash, std::equal_to<>> m_SceneGuidsByName;

		std::shared_ptr<FileSystem> m_FileSystem;
		ProjectManifestData m_ProjectManifest{};
		DatFileHeader m_Header{ c_PackageDatFormat };
	};
}
