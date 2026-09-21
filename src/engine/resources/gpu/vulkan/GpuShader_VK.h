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

		GpuShader_VK(const Guid& guid, std::string name, ResourceRef<CpuShader> cpuShader);
		~GpuShader_VK() override = default;

		[[nodiscard]] static std::shared_ptr<GpuShader_VK> CreateFromCpu(ResourceRef<CpuShader> cpuShader);

		[[nodiscard]] const ResourceRef<CpuShader>& GetCpuShader() const noexcept { return m_CpuShader; }

	private:
		ResourceRef<CpuShader> m_CpuShader;
	};
}
