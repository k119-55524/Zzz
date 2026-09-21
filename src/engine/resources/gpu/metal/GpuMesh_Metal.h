#pragma once

#include <string>
#include <memory>

#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "engine/resources/ResourceBase.h"

using namespace zzz::core;

namespace zzz::engine
{
	class GpuMesh_Metal final : public ResourceBase
	{
	public:
		using CpuSource = CpuMesh;

		GpuMesh_Metal(const Guid& guid, std::string name, ResourceRef<CpuMesh> cpuMesh)
			: ResourceBase(guid, eResourceType::Mesh, std::move(name)), m_CpuMesh(std::move(cpuMesh)) {}
		~GpuMesh_Metal() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMesh_Metal> CreateFromCpu(ResourceRef<CpuMesh> cpuMesh)
		{
			return safe_make_shared<GpuMesh_Metal>(cpuMesh->GetGuid(), std::string(cpuMesh->GetName()), std::move(cpuMesh));
		}

		[[nodiscard]] const ResourceRef<CpuMesh>& GetCpuMesh() const noexcept { return m_CpuMesh; }
		[[nodiscard]] zU32 GetVertexCount() const noexcept { return m_CpuMesh ? m_CpuMesh->GetVertexCount() : 0; }
		[[nodiscard]] zU32 GetIndexCount() const noexcept { return m_CpuMesh ? m_CpuMesh->GetIndexCount() : 0; }

	private:
		ResourceRef<CpuMesh> m_CpuMesh;
	};
}
