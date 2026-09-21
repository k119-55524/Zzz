#include "GpuMesh_VK.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuMesh_VK::GpuMesh_VK(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuMesh> cpuMesh)
		: ResourceBase(guid, eResourceType::Mesh, std::move(name))
		, m_CpuMesh(std::move(cpuMesh))
	{
	}

	std::shared_ptr<GpuMesh_VK> GpuMesh_VK::CreateFromCpu(ResourceRef<CpuMesh> cpuMesh)
	{
		ensure(cpuMesh != nullptr, "GpuMesh_VK::CreateFromCpu: cpuMesh не должен быть null");
		const auto& guid = cpuMesh->GetGuid();
		std::string name(cpuMesh->GetName());
		return safe_make_shared<GpuMesh_VK>(guid, std::move(name), std::move(cpuMesh));
	}
}
