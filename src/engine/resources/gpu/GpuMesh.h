#pragma once

#include <string>
#include <memory>

#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuMesh.h"
#include "engine/resources/ResourceBase.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class GpuMesh
	 * @brief Ресурс полигональной 3D-геометрии на GPU.
	 * @details На этапе 22 хранит валидную ссылку на CPU-ресурс и дескрипторы.
	 *          Настоящие Vertex/Index GPU-буферы и барьеры создаются на этапе 23.
	 */
	class GpuMesh final : public ResourceBase
	{
	public:
		using CpuSource = CpuMesh;

		GpuMesh(const Guid& guid, std::string name, ResourceRef<CpuMesh> cpuMesh);
		~GpuMesh() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMesh> CreateFromCpu(ResourceRef<CpuMesh> cpuMesh);

		[[nodiscard]] const ResourceRef<CpuMesh>& GetCpuMesh() const noexcept { return m_CpuMesh; }
		[[nodiscard]] zU32 GetVertexCount() const noexcept { return m_CpuMesh ? m_CpuMesh->GetVertexCount() : 0; }
		[[nodiscard]] zU32 GetIndexCount() const noexcept { return m_CpuMesh ? m_CpuMesh->GetIndexCount() : 0; }

	private:
		ResourceRef<CpuMesh> m_CpuMesh;
	};
}
