
#include <format>
#include <logger.h>

#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/package/PackageManager.h"
#include "core/io/ResourceStorageTraits.h"
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
		std::shared_ptr<FileSystem> fileSystem) :
			m_TaskDispatcher(taskDispatcher),
			m_PackageManager(std::move(packageManager)),
			m_DataAssetsManager(std::move(dataAssetsManager)),
			m_FileSystem(std::move(fileSystem)),
			m_IoScheduler(m_FileSystem ? safe_make_unique<IoScheduler>(m_FileSystem) : nullptr)
	{
		ensure(m_PackageManager != nullptr, "PackageManager не должен быть null в CpuResourceManager.");
	}

	CpuResourceManager::~CpuResourceManager()
	{
		m_IoScheduler = nullptr;
	}

	std::expected<PackageEntry, std::string> CpuResourceManager::FindEntry(const Guid& guid, eResourceType type) const
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
}

