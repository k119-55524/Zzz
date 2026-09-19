#pragma once

#include <memory>
#include <string>
#include "engine/resources/ResourceBase.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuMaterial.h"

namespace zzz::engine
{
	/**
	 * @class GpuMaterial
	 * @brief Ресурс материала на GPU (шейдеры, параметры, текстурные привязки).
	 */
	class GpuMaterial final : public ResourceBase
	{
	public:
		using CpuType = CpuMaterial;

		GpuMaterial(
			const ::zzz::core::Guid& guid,
			std::string name,
			ResourceRef<CpuMaterial> cpuMaterial);
		~GpuMaterial() override = default;

		[[nodiscard]] const ResourceRef<CpuMaterial>& GetCpuMaterial() const noexcept { return m_CpuMaterial; }
		[[nodiscard]] const ::zzz::core::Guid& GetShaderGuid() const noexcept
		{
			static const ::zzz::core::Guid s_Empty;
			return m_CpuMaterial ? m_CpuMaterial->GetShaderGuid() : s_Empty;
		}

	private:
		ResourceRef<CpuMaterial> m_CpuMaterial;
	};
}
