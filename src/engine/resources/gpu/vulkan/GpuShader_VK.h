#pragma once

#include <string>
#include <memory>
#include "engine/resources/ResourceRef.h"
#include "engine/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuShader.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class GpuShader_VK
	 * @brief Ресурс скомпилированного шейдера на GPU (Vulkan: VkShaderModule).
	 */
	class GpuShader_VK final : public ResourceBase
	{
	public:
		using CpuSource = CpuShader;

		GpuShader_VK(const Guid& guid, std::string name);
		~GpuShader_VK() override = default;

		[[nodiscard]] static std::shared_ptr<GpuShader_VK> CreateGpuResourceAndUploadFromCpu(ResourceRef<CpuShader> cpuShader);
	};
}
