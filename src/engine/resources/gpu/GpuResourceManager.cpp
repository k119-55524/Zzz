
#include "core/utils/Ensure.h"

#include "GpuResourceManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuResourceManager::GpuResourceManager(
		TaskDispatcher& taskDispatcher,
		std::shared_ptr<GAPI> gapi,
		std::shared_ptr<CpuResourceManager> cpuResourceManager)
		: m_TaskDispatcher(taskDispatcher)
		, m_GAPI(std::move(gapi))
		, m_CpuManager(std::move(cpuResourceManager))
		, m_Meshes([this](auto task) { m_MainThreadQueue.Push(std::move(task)); })
		, m_Materials([this](auto task) { m_MainThreadQueue.Push(std::move(task)); })
		, m_Textures([this](auto task) { m_MainThreadQueue.Push(std::move(task)); })
		, m_Shaders([this](auto task) { m_MainThreadQueue.Push(std::move(task)); })
	{
		ensure(m_CpuManager != nullptr, "CpuResourceManager не должен быть null в GpuResourceManager.");
	}

	GpuResourceManager::~GpuResourceManager()
	{
		Stop();
		Clear();
	}

	void GpuResourceManager::Stop()
	{
		m_IsRunning.store(false, std::memory_order_release);
		m_MainThreadQueue.Clear();
	}

	void GpuResourceManager::Clear()
	{
		EmergencyStop();
	}

	void GpuResourceManager::EmergencyStop()
	{
		m_MainThreadQueue.Clear();
		m_Meshes.Clear();
		m_Materials.Clear();
		m_Textures.Clear();
		m_Shaders.Clear();
	}
}
