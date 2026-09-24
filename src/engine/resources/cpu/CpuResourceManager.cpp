#include "core/utils/Ensure.h"

#include "CpuResourceManager.h"

Z_SET_LOG_CATEGORY(::zzz::core::LogEngine);

using namespace zzz::core;
namespace zzz::engine
{
	CpuResourceManager::CpuResourceManager(
		TaskDispatcher& taskDispatcher,
		std::shared_ptr<DataAssetsManager> dataAssetsManager) :
			m_TaskDispatcher(taskDispatcher),
			m_DataAssetsManager(std::move(dataAssetsManager))
	{
		ensure(m_DataAssetsManager != nullptr, "DataAssetsManager не должен быть null в CpuResourceManager.");
	}

	CpuResourceManager::~CpuResourceManager()
	{
		std::unique_lock lock(m_ShutdownMutex);
		m_IsStopping.store(true, std::memory_order_release);
		m_ShutdownCv.wait(lock, [this]() {
			return m_ActiveIoTasks == 0;
		});
	}
}
