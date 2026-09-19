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
}
