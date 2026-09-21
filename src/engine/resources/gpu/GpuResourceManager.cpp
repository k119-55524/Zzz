
#include "core/utils/Ensure.h"

#include "GpuResourceManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuResourceManager::GpuResourceManager(
		TaskDispatcher& taskDispatcher,
		std::shared_ptr<GAPI> gapi,
		std::shared_ptr<CpuResourceManager> cpuResourceManager) :
			m_TaskDispatcher(taskDispatcher),
			m_GAPI(std::move(gapi)),
			m_CpuManager(std::move(cpuResourceManager))
	{
		ensure(m_CpuManager != nullptr, "CpuResourceManager не должен быть null в GpuResourceManager.");
	}

	GpuResourceManager::~GpuResourceManager()
	{
		EmergencyStop();
	}

	void GpuResourceManager::EmergencyStop()
	{
		m_Meshes.Clear();
		m_Materials.Clear();
		m_Textures.Clear();
		m_Shaders.Clear();
	}
}
