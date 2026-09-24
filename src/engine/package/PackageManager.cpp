
#include "core/utils/Ensure.h"
#include "core/io/package/scene/SceneData.h"
#include "core/io/package/assets/PrefabData.h"
#include "core/io/package/views/ChildViewData.h"
#include "core/io/package/views/IndependentViewData.h"

#include "PackageManager.h"

using namespace zzz::engine;
using namespace zzz::core;

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace
{
	[[nodiscard]] constexpr bool IsGamePackageResourceType(ePackage pkgType) noexcept
	{
		switch (pkgType)
		{
		case ePackage::ProjectManifest:
		case ePackage::Scene:
		case ePackage::PrimaryView:
		case ePackage::ChildView:
		case ePackage::IndependentView:
		case ePackage::Prefab:
			return true;
		default:
			return false;
		}
	}
}

namespace zzz::engine
{
	PackageManager::PackageManager(std::shared_ptr<FileSystem> fileSystem)
		: PackageArchive(std::move(fileSystem))
	{
		InitializeArchive(ArchiveInitParams<ePackage>{
			.relativePath = c_GamePackageRelativePath,
			.expectedFormat = c_PackageDatFormat,
			.isTypeAllowed = &IsGamePackageResourceType
		});

		const std::string pathStr = c_GamePackageRelativePath.generic_string();

		auto primaryViewIt = m_EntriesByGuid.find(ePackage::PrimaryView);
		if (primaryViewIt == m_EntriesByGuid.end() || primaryViewIt->second.empty())
			THROW_RUNTIME("Ошибка пакета '{}': Обязательный ресурс PrimaryViewData отсутствует.", pathStr);

		auto manifestIt = m_EntriesByGuid.find(ePackage::ProjectManifest);
		if (manifestIt == m_EntriesByGuid.end() || manifestIt->second.empty())
			THROW_RUNTIME("Ошибка пакета '{}': Обязательный ресурс ProjectManifestData отсутствует.", pathStr);

		auto manifestRes = DeserializeEntryRaw<ProjectManifestData>(manifestIt->second.begin()->second);
		if (!manifestRes)
			THROW_RUNTIME("Ошибка десериализации ProjectManifestData из пакета '{}': {}", pathStr, manifestRes.error());

		if (manifestRes->GetCompanyName().empty() || manifestRes->GetAppName().empty())
			THROW_RUNTIME("Ошибка пакета '{}': ProjectManifestData не содержит имя компании и/или приложения.", pathStr);

		m_ProjectManifest = std::move(*manifestRes);

		for (const auto& sceneEntry : m_ProjectManifest.GetScenes())
		{
#if Z_DEBUG_BUILD || Z_DEVELOPMENT_BUILD
			if (auto it = m_SceneGuidsByName.find(sceneEntry.GetName()); it != m_SceneGuidsByName.end())
			{
				ensure(false,
					"Обнаружен дубликат имени сцены '{}' в манифесте проекта package.dat (существующий GUID={}, дублирующий GUID={})!",
					sceneEntry.GetName(),
					it->second.ToString(),
					sceneEntry.GetGuid().ToString());
			}
#endif
			m_SceneGuidsByName.emplace(sceneEntry.GetName(), sceneEntry.GetGuid());
		}
	}

	std::optional<Guid> PackageManager::FindSceneGuidByName(std::string_view name) const noexcept
	{
		auto it = m_SceneGuidsByName.find(name);
		if (it != m_SceneGuidsByName.end())
			return it->second;

		return std::nullopt;
	}

	std::expected<PrimaryViewData, std::string> PackageManager::GetPrimaryViewData() const
	{
		auto it = m_EntriesByGuid.find(ePackage::PrimaryView);
		if (it == m_EntriesByGuid.end() || it->second.empty())
			return UNEXPECTED("Package entry of type PrimaryView was not found.");

		return DeserializeEntryRaw<PrimaryViewData>(it->second.begin()->second);
	}

	void PackageManager::LogEntryDetails(const PackageEntry& entry) const
	{
#if Z_ADD_LOGGER
		const auto pkgType = static_cast<ePackage>(entry.GetAssetType());
		switch (pkgType)
		{
		case ePackage::ProjectManifest:
			if (auto res = DeserializeEntryRaw<ProjectManifestData>(entry))
				res->LogFileBlock("      ");
			break;
		case ePackage::PrimaryView:
			if (auto res = DeserializeEntryRaw<PrimaryViewData>(entry))
				res->LogFileBlock("      ");
			break;
		case ePackage::Scene:
			if (auto res = DeserializeEntryRaw<SceneData>(entry))
				res->LogFileBlock("      ");
			break;
		case ePackage::ChildView:
			if (auto res = DeserializeEntryRaw<ChildViewData>(entry))
				res->LogFileBlock("      ");
			break;
		case ePackage::IndependentView:
			if (auto res = DeserializeEntryRaw<IndependentViewData>(entry))
				res->LogFileBlock("      ");
			break;
		case ePackage::Prefab:
			if (auto res = DeserializeEntryRaw<PrefabData>(entry))
				res->LogFileBlock("      ");
			break;
		}
#endif // Z_ADD_LOGGER
	}
}
