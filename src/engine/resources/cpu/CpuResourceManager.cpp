
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/tasks/TaskDispatcher.h"

#include "CpuResourceManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	CpuResourceManager::CpuResourceManager(
		TaskDispatcher& taskDispatcher,
		std::shared_ptr<DataAssetsManager> dataAssetsManager,
		std::shared_ptr<FileSystem> fileSystem) :
			m_TaskDispatcher(taskDispatcher),
			m_DataAssetsManager(std::move(dataAssetsManager)),
			m_FileSystem(std::move(fileSystem)),
			m_IoScheduler(safe_make_unique<IoScheduler>(m_FileSystem))
	{
		ensure(m_DataAssetsManager != nullptr, "DataAssetsManager не должен быть null в CpuResourceManager.");
		ensure(m_FileSystem != nullptr, "FileSystem не должен быть null в CpuResourceManager.");
	}

	CpuResourceManager::~CpuResourceManager()
	{
		m_IoScheduler = nullptr;
	}
}
