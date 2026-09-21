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

		GpuMaterial_VK(const Guid& guid, std::string name, ResourceRef<CpuMaterial> cpuMaterial);
		~GpuMaterial_VK() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMaterial_VK> CreateFromCpu(ResourceRef<CpuMaterial> cpuMaterial);

		[[nodiscard]] const ResourceRef<CpuMaterial>& GetCpuMaterial() const noexcept { return m_CpuMaterial; }
		[[nodiscard]] const Guid& GetShaderGuid() const noexcept
		{
			static const Guid s_Empty;
			return m_CpuMaterial ? m_CpuMaterial->GetShaderGuid() : s_Empty;
		}

	private:
		ResourceRef<CpuMaterial> m_CpuMaterial;
	};
}
