#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "GpuMesh.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuMesh::GpuMesh(
		const Guid& guid,
		std::string name,
		ResourceRef<CpuMesh> cpuMesh)
		: ResourceBase(guid, eResourceType::Mesh, std::move(name))
		, m_CpuMesh(std::move(cpuMesh))
	{
	}

	std::shared_ptr<GpuMesh> GpuMesh::CreateFromCpu(ResourceRef<CpuMesh> cpuMesh)
	{
		ensure(cpuMesh != nullptr, "GpuMesh::CreateFromCpu: cpuMesh не должен быть null");
		const auto& guid = cpuMesh->GetGuid();
		std::string name(cpuMesh->GetName());
		return safe_make_shared<GpuMesh>(guid, std::move(name), std::move(cpuMesh));
	}
}
