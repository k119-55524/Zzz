#include "GpuMesh_VK.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

namespace zzz::engine
{
	GpuMesh_VK::GpuMesh_VK(
		const Guid& guid,
		std::string name,
		zU32 vertexCount,
		zU32 indexCount)
		: ResourceBase(guid, eResourceType::Mesh, std::move(name))
		, m_VertexCount(vertexCount)
		, m_IndexCount(indexCount)
	{
	}

	std::shared_ptr<GpuMesh_VK> GpuMesh_VK::CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuMesh> cpuMesh)
	{
		ensure(cpuMesh != nullptr, "GpuMesh_VK::CreateGpuResourceAndUploadFromCpu: cpuMesh не должен быть null");
		const auto& guid = cpuMesh->GetGuid();
		std::string name(cpuMesh->GetName());
		const zU32 vertexCount = cpuMesh->GetVertexCount();
		const zU32 indexCount = cpuMesh->GetIndexCount();

		return safe_make_shared<GpuMesh_VK>(guid, std::move(name), vertexCount, indexCount);
	}
}
