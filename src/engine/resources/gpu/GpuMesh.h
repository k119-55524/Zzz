#pragma once

#include <memory>
#include <string>
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuMesh.h"

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
		using CpuType = CpuMesh;

		GpuMesh(
			const ::zzz::core::Guid& guid,
			std::string name,
			std::shared_ptr<CpuMesh> cpuMesh);
		~GpuMesh() override = default;

		[[nodiscard]] const std::shared_ptr<CpuMesh>& GetCpuMesh() const noexcept { return m_CpuMesh; }
		[[nodiscard]] zU32 GetVertexCount() const noexcept { return m_CpuMesh ? m_CpuMesh->GetVertexCount() : 0; }
		[[nodiscard]] zU32 GetIndexCount() const noexcept { return m_CpuMesh ? m_CpuMesh->GetIndexCount() : 0; }

	private:
		std::shared_ptr<CpuMesh> m_CpuMesh;
	};
}
