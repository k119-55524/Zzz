#include <format>
#include <logger.h>

#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/package/PackageManager.h"
#include "core/io/ResourceStorageTraits.h"
#include "core/io/package/DataAssetsManager.h"
#include "engine/resources/cpu/loaders/CpuMeshLoader.h"
#include "engine/resources/cpu/loaders/CpuMaterialLoader.h"
#include "engine/resources/cpu/loaders/CpuShaderLoader.h"

#include "CpuResourceManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	template<typename... SupportedResources>
	std::expected<PackageEntry, std::string> CpuResourceManager<SupportedResources...>::FindEntry(
		const Guid& guid,
		eResourceType type) const
	{
		const auto storageKind = GetResourceStorageTraits(type).storageKind;
		if (storageKind == eResourceStorageKind::DataArchive)
		{
			if (!m_DataAssetsManager)
			{
				return std::unexpected("DataAssetsManager не инициализирован");
			}
			auto entryOpt = m_DataAssetsManager->GetEntry(type, guid);
			if (entryOpt.has_value())
			{
				return *entryOpt;
			}
		}
		else if (storageKind == eResourceStorageKind::PackageArchive)
		{
			if (!m_PackageManager)
			{
				return std::unexpected("PackageManager не инициализирован");
			}
			auto entryOpt = m_PackageManager->GetEntry(guid);
			if (entryOpt.has_value())
			{
				return *entryOpt;
			}
		}

		return std::unexpected(std::format("Запись ресурса с GUID '{}' не найдена в хранилище", guid.ToString()));
	}

	template<>
	std::expected<std::shared_ptr<CpuMesh>, std::string> LoadCpuResourceSync<CpuMesh, CoreCpuResourceManager>(
		const Guid& guid,
		CoreCpuResourceManager& manager)
	{
		auto entryRes = manager.FindEntry(guid, eResourceType::Mesh);
		if (!entryRes)
		{
			return std::unexpected(entryRes.error());
		}
		auto dataAssetsMgr = manager.m_DataAssetsManager.get();
		if (!dataAssetsMgr)
		{
			return std::unexpected("DataAssetsManager не инициализирован");
		}
		return CpuMeshLoader::Load(*entryRes, *dataAssetsMgr);
	}

	template<>
	std::expected<std::shared_ptr<CpuMaterial>, std::string> LoadCpuResourceSync<CpuMaterial, CoreCpuResourceManager>(
		const Guid& guid,
		CoreCpuResourceManager& manager)
	{
		auto entryRes = manager.FindEntry(guid, eResourceType::Material);
		if (!entryRes)
		{
			return std::unexpected(entryRes.error());
		}
		auto dataAssetsMgr = manager.m_DataAssetsManager.get();
		if (!dataAssetsMgr)
		{
			return std::unexpected("DataAssetsManager не инициализирован");
		}
		return CpuMaterialLoader::Load(*entryRes, *dataAssetsMgr);
	}

	template<>
	std::expected<std::shared_ptr<CpuShader>, std::string> LoadCpuResourceSync<CpuShader, CoreCpuResourceManager>(
		const Guid& guid,
		CoreCpuResourceManager& manager)
	{
		auto entryRes = manager.FindEntry(guid, eResourceType::Shader);
		if (!entryRes)
		{
			return std::unexpected(entryRes.error());
		}
		auto dataAssetsMgr = manager.m_DataAssetsManager.get();
		if (!dataAssetsMgr)
		{
			return std::unexpected("DataAssetsManager не инициализирован");
		}
		return CpuShaderLoader::Load(*entryRes, *dataAssetsMgr);
	}

	template<>
	std::expected<std::shared_ptr<CpuTexture2D>, std::string> LoadCpuResourceSync<CpuTexture2D, CoreCpuResourceManager>(
		const Guid& guid,
		CoreCpuResourceManager& manager)
	{
		auto entryRes = manager.FindEntry(guid, eResourceType::Texture2D);
		if (!entryRes)
		{
			return std::unexpected(entryRes.error());
		}
		return safe_make_shared<CpuTexture2D>(entryRes->GetGuid(), std::string(entryRes->GetName()));
	}

	template<typename... SupportedResources>
	std::expected<SceneData, std::string> CpuResourceManager<SupportedResources...>::LoadSceneData(const Guid& sceneGuid)
	{
		auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneGuid);
		if (!entryOpt.has_value())
		{
			return std::unexpected(std::format("Сцена с GUID '{}' не найдена в package.dat.", sceneGuid.ToString()));
		}

		auto sceneDataRes = m_PackageManager->LoadAsset<SceneData>(sceneGuid);
		if (!sceneDataRes.has_value())
		{
			return std::unexpected(std::format("Ошибка загрузки данных сцены '{}' ({}): {}",
				entryOpt->GetName(), sceneGuid.ToString(), sceneDataRes.error()));
		}

		return std::move(*sceneDataRes);
	}

	template<typename... SupportedResources>
	std::expected<SceneData, std::string> CpuResourceManager<SupportedResources...>::LoadSceneData(std::string_view sceneName)
	{
		auto entryOpt = m_PackageManager->GetEntry(ePackage::Scene, sceneName);
		if (!entryOpt.has_value())
		{
			return std::unexpected(std::format("Сцена с именем '{}' не найдена в package.dat.", sceneName));
		}

		return LoadSceneData(entryOpt->GetGuid());
	}

	template class CpuResourceManager<CpuMesh, CpuMaterial, CpuTexture2D, CpuShader>;
}
