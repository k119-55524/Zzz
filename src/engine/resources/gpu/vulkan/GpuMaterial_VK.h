#pragma once

#include <string>
#include <memory>
#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuMaterial.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class GpuMaterial_VK
	 * @brief Ресурс материала на GPU (Vulkan: пайплайны, дескрипторные сеты, текстурные привязки).
	 */
	class GpuMaterial_VK final : public ResourceBase
	{
	public:
		using CpuSource = CpuMaterial;

		GpuMaterial_VK(const Guid& guid, std::string name, const Guid& shaderGuid);
		~GpuMaterial_VK() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMaterial_VK> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuMaterial> cpuMaterial);

		[[nodiscard]] const Guid& GetShaderGuid() const noexcept { return m_ShaderGuid; }

	private:
		Guid m_ShaderGuid;
	};
}
