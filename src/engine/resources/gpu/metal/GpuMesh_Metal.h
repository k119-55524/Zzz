#pragma once

#include <string>
#include <memory>
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"

using namespace zzz::core;

namespace zzz::engine
{
	class GpuMesh_Metal final : public ResourceBase
	{
	public:
		using CpuSource = CpuMesh;

		GpuMesh_Metal(const Guid& guid, std::string name, zU32 vertexCount, zU32 indexCount)
			: ResourceBase(guid, eEngineResourceType::Mesh, std::move(name))
			, m_VertexCount(vertexCount)
			, m_IndexCount(indexCount)
		{
		}

		~GpuMesh_Metal() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMesh_Metal> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuMesh> cpuMesh)
		{
			ensure(cpuMesh != nullptr, "GpuMesh_Metal::CreateGpuResourceAndUploadFromCpu: cpuMesh не должен быть null");
			const auto& guid = cpuMesh->GetGuid();
			std::string name(cpuMesh->GetName());
			const zU32 vertexCount = cpuMesh->GetVertexCount();
			const zU32 indexCount = cpuMesh->GetIndexCount();

			return safe_make_shared<GpuMesh_Metal>(guid, std::move(name), vertexCount, indexCount);
		}

		[[nodiscard]] zU32 GetVertexCount() const noexcept { return m_VertexCount; }
		[[nodiscard]] zU32 GetIndexCount() const noexcept { return m_IndexCount; }

	private:
		zU32 m_VertexCount{ 0 };
		zU32 m_IndexCount{ 0 };
	};
}
