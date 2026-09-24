#pragma once

#include <string>
#include <expected>
#include <string_view>
#include <unordered_map>

#include "core/enums/ePackageDatType.h"
#include "core/io/package/PackageEntry.h"
#include "core/io/package/ArchiveReaderBase.h"
#include "core/io/package/ProjectManifestData.h"
#include "core/io/package/views/PrimaryViewData.h"

namespace zzz::engine
{
	using namespace zzz::core;
	class PackageManager final : public ArchiveReaderBase<ePackageDatType>
	{
	public:
		PackageManager() = delete;
		explicit PackageManager(const std::filesystem::path& physicalPath);
		~PackageManager() override = default;

		using ArchiveReaderBase<ePackageDatType>::GetEntry;
		using ArchiveReaderBase<ePackageDatType>::HasEntry;

		/// @brief Поиск записи по общему типу ресурса движка.
		[[nodiscard]] const PackageEntry* GetEntry(eEngineResourceType type, const Guid& guid) const noexcept
		{
			const auto packageDatType = TryToPackageDatType(type);
			if (!packageDatType)
				return nullptr;
			return ArchiveReaderBase<ePackageDatType>::GetEntry(*packageDatType, guid);
		}

		[[nodiscard]] bool HasEntry(eEngineResourceType type, const Guid& guid) const noexcept
		{
			return GetEntry(type, guid) != nullptr;
		}

		[[nodiscard]] const ProjectManifestData& GetProjectManifestData() const noexcept { return m_ProjectManifest; }
		[[nodiscard]] std::expected<PrimaryViewData, std::string> GetPrimaryViewData() const;

		/// @brief Имя компании и приложения из ProjectManifestData.
		[[nodiscard]] const std::string& GetCompanyName() const noexcept { return m_ProjectManifest.GetCompanyName(); }
		[[nodiscard]] const std::string& GetAppName() const noexcept { return m_ProjectManifest.GetAppName(); }

		template <typename T>
		[[nodiscard]] std::expected<T, std::string> LoadAsset(const Guid& guid) const
		{
			static_assert(requires { { T::c_PackageType } -> std::convertible_to<ePackageDatType>; },
				"T must define static constexpr ePackageDatType c_PackageType");
			static_assert(ArchiveTraits<ePackageDatType>::IsTypeAllowed(T::c_PackageType),
				"Asset type is not allowed in package.dat");

			constexpr ePackageDatType type = T::c_PackageType;
			const auto* entry = GetEntry(type, guid);
			if (!entry)
				return UNEXPECTED("Package entry of type {} with GUID '{}' was not found.", ToString(type), guid.ToString());

			return DeserializeEntryRaw<T>(*entry);
		}

		[[nodiscard]] std::optional<Guid> FindSceneGuidByName(std::string_view name) const noexcept;

	private:
		void LogEntryDetails(const PackageEntry& entry) const override;

		struct StringHash
		{
			using is_transparent = void;
			[[nodiscard]] size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
		};
		std::unordered_map<std::string, Guid, StringHash, std::equal_to<>> m_SceneGuidsByName;
		ProjectManifestData m_ProjectManifest{};
	};
}
