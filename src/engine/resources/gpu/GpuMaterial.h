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
	 * @class GpuMaterial
	 * @brief Ресурс материала на GPU (шейдеры, параметры, текстурные привязки).
	 */
	class GpuMaterial final : public ResourceBase
	{
	public:
		using CpuSource = CpuMaterial;

		GpuMaterial(const Guid& guid, std::string name, ResourceRef<CpuMaterial> cpuMaterial);
		~GpuMaterial() override = default;

		[[nodiscard]] static std::shared_ptr<GpuMaterial> CreateFromCpu(ResourceRef<CpuMaterial> cpuMaterial);

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
