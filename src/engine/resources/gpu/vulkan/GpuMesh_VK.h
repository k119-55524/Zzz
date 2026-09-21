#pragma once

#include <string>
#include <memory>
#include "engine/resources/cpu/CpuMesh.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class GpuMesh_VK
	 * @brief Ресурс полигональной 3D-геометрии на GPU (Vulkan).
	 * @details На этапе 22 хранит валидные дескрипторы геометрии.
	 *          Настоящие Vertex/Index GPU-буферы (VkBuffer) и барьеры создаются на этапе 23.
	 */
	class GpuMesh_VK final : public ResourceBase
	{
	public:
		using CpuSource = CpuMesh;

		GpuMesh_VK(const Guid& guid, std::string name, zU32 vertexCount, zU32 indexCount);
		~GpuMesh_VK() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMesh_VK> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuMesh> cpuMesh);

		[[nodiscard]] zU32 GetVertexCount() const noexcept { return m_VertexCount; }
		[[nodiscard]] zU32 GetIndexCount() const noexcept { return m_IndexCount; }

	private:
		zU32 m_VertexCount{ 0 };
		zU32 m_IndexCount{ 0 };
	};
}
