#include "engine/gapi/GAPI.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/tasks/TaskDispatcher.h"
#include "engine/resources/cpu/CpuResourceManager.h"

#include "GpuResourceManager.h"

using namespace zzz::core;
using namespace zzz::templates;

namespace zzz::engine
{
	std::shared_ptr<GpuMesh> CreateGpuResource(std::shared_ptr<CpuMesh> cpuMesh, GAPI*)
	{
		if (!cpuMesh)
		{
			return nullptr;
		}
		// На этапе 22 - сохранение ссылки на CPU-меш. На этапе 23 - создание аппаратных Vertex/Index буферов GAPI.
		return safe_make_shared<GpuMesh>(cpuMesh->GetGuid(), std::string(cpuMesh->GetName()), std::move(cpuMesh));
	}

	std::shared_ptr<GpuMaterial> CreateGpuResource(std::shared_ptr<CpuMaterial> cpuMaterial, GAPI*)
	{
		if (!cpuMaterial)
		{
			return nullptr;
		}
		return safe_make_shared<GpuMaterial>(cpuMaterial->GetGuid(), std::string(cpuMaterial->GetName()), std::move(cpuMaterial));
	}

	std::shared_ptr<GpuTexture2D> CreateGpuResource(std::shared_ptr<CpuTexture2D> cpuTexture, GAPI*)
	{
		if (!cpuTexture)
		{
			return nullptr;
		}
		return safe_make_shared<GpuTexture2D>(cpuTexture->GetGuid(), std::string(cpuTexture->GetName()), std::move(cpuTexture));
	}

	std::shared_ptr<GpuShader> CreateGpuResource(std::shared_ptr<CpuShader> cpuShader, GAPI*)
	{
		if (!cpuShader)
		{
			return nullptr;
		}
		return safe_make_shared<GpuShader>(cpuShader->GetGuid(), std::string(cpuShader->GetName()), std::move(cpuShader));
	}

	template class GpuResourceManager<GpuMesh, GpuMaterial, GpuTexture2D, GpuShader>;
}
