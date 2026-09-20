
#include <format>
#include <logger.h>

#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/package/PackageManager.h"
#include "core/io/ResourceStorageTraits.h"
#include "core/io/package/DataAssetsManager.h"
#include "engine/resources/cpu/loaders/CpuMeshLoader.h"
#include "engine/resources/cpu/loaders/CpuShaderLoader.h"
#include "engine/resources/cpu/loaders/CpuMaterialLoader.h"

#include "CpuResourceManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	CpuResourceManager::CpuResourceManager(
		TaskDispatcher& taskDispatcher,
		std::shared_ptr<PackageManager> packageManager,
		std::shared_ptr<DataAssetsManager> dataAssetsManager,
		std::shared_ptr<FileSystem> fileSystem)
		: m_TaskDispatcher(taskDispatcher)
		, m_PackageManager(std::move(packageManager))
		, m_DataAssetsManager(std::move(dataAssetsManager))
		, m_FileSystem(std::move(fileSystem))
		, m_IoScheduler(m_FileSystem ? safe_make_unique<IoScheduler>(m_FileSystem) : nullptr)
		, m_Meshes([this](auto task) { m_TaskDispatcher.Submit(eTaskPriority::High, std::move(task)); })
		, m_Materials([this](auto task) { m_TaskDispatcher.Submit(eTaskPriority::High, std::move(task)); })
		, m_Textures([this](auto task) { m_TaskDispatcher.Submit(eTaskPriority::High, std::move(task)); })
		, m_Shaders([this](auto task) { m_TaskDispatcher.Submit(eTaskPriority::High, std::move(task)); })
	{
		ensure(m_PackageManager != nullptr, "PackageManager не должен быть null в CpuResourceManager.");
	}

	CpuResourceManager::~CpuResourceManager()
	{
		Stop();
		Clear();
	}

	void CpuResourceManager::Stop()
	{
		m_IsRunning.store(false, std::memory_order_release);
		if (m_IoScheduler)
		{
			m_IoScheduler->Stop();
		}
	}

	void CpuResourceManager::Clear()
	{
		EmergencyStop();
	}

	void CpuResourceManager::EmergencyStop()
	{
		m_Meshes.Clear();
		m_Materials.Clear();
		m_Textures.Clear();
		m_Shaders.Clear();
	}

	std::expected<PackageEntry, std::string> CpuResourceManager::FindEntry(
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
	std::expected<std::shared_ptr<CpuMesh>, std::string> CpuResourceManager::LoadResourceSync<CpuMesh>(const Guid& guid)
	{
		auto entryRes = FindEntry(guid, eResourceType::Mesh);
		if (!entryRes)
		{
			return std::unexpected(entryRes.error());
		}
		if (!m_DataAssetsManager)
		{
			return std::unexpected("DataAssetsManager не инициализирован");
		}
		return CpuMeshLoader::Load(*entryRes, *m_DataAssetsManager);
	}

	template<>
	std::expected<std::shared_ptr<CpuMaterial>, std::string> CpuResourceManager::LoadResourceSync<CpuMaterial>(const Guid& guid)
	{
		auto entryRes = FindEntry(guid, eResourceType::Material);
		if (!entryRes)
		{
			return std::unexpected(entryRes.error());
		}
		if (!m_DataAssetsManager)
		{
			return std::unexpected("DataAssetsManager не инициализирован");
		}
		return CpuMaterialLoader::Load(*entryRes, *m_DataAssetsManager);
	}

	template<>
	std::expected<std::shared_ptr<CpuShader>, std::string> CpuResourceManager::LoadResourceSync<CpuShader>(const Guid& guid)
	{
		auto entryRes = FindEntry(guid, eResourceType::Shader);
		if (!entryRes)
		{
			return std::unexpected(entryRes.error());
		}
		if (!m_DataAssetsManager)
		{
			return std::unexpected("DataAssetsManager не инициализирован");
		}
		return CpuShaderLoader::Load(*entryRes, *m_DataAssetsManager);
	}

	template<>
	std::expected<std::shared_ptr<CpuTexture2D>, std::string> CpuResourceManager::LoadResourceSync<CpuTexture2D>(const Guid& guid)
	{
		auto entryRes = FindEntry(guid, eResourceType::Texture2D);
		if (!entryRes)
		{
			return std::unexpected(entryRes.error());
		}
		return safe_make_shared<CpuTexture2D>(entryRes->GetGuid(), std::string(entryRes->GetName()));
	}

	template<>
	std::expected<std::shared_ptr<CpuMesh>, std::string> CpuResourceManager::LoadFromBytes<CpuMesh>(
		const PackageEntry& entry,
		std::span<const std::byte> bytes)
	{
		return CpuMeshLoader::LoadFromMemory(entry, bytes);
	}

	template<>
	std::expected<std::shared_ptr<CpuMaterial>, std::string> CpuResourceManager::LoadFromBytes<CpuMaterial>(
		const PackageEntry& entry,
		std::span<const std::byte> bytes)
	{
		return CpuMaterialLoader::LoadFromMemory(entry, bytes);
	}

	template<>
	std::expected<std::shared_ptr<CpuShader>, std::string> CpuResourceManager::LoadFromBytes<CpuShader>(
		const PackageEntry& entry,
		std::span<const std::byte> bytes)
	{
		return CpuShaderLoader::LoadFromMemory(entry, bytes);
	}

	template<>
	std::expected<std::shared_ptr<CpuTexture2D>, std::string> CpuResourceManager::LoadFromBytes<CpuTexture2D>(
		const PackageEntry& entry,
		std::span<const std::byte> /*bytes*/)
	{
		return safe_make_shared<CpuTexture2D>(entry.GetGuid(), std::string(entry.GetName()));
	}
}

