
#include "core/utils/Ensure.h"

#include "GpuResourceManager.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuResourceManager::GpuResourceManager(
		std::shared_ptr<GAPI> gapi,
		std::shared_ptr<CpuResourceManager> cpuResourceManager) :
			m_GAPI(std::move(gapi)),
			m_CpuManager(std::move(cpuResourceManager))
	{
		ensure(m_CpuManager != nullptr, "CpuResourceManager не должен быть null в GpuResourceManager.");
	}

}
