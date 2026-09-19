#pragma once

#include <memory>
#include <string>
#include "engine/resources/ResourceBase.h"
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
		GpuMaterial(
			const ::zzz::core::Guid& guid,
			std::string name,
			std::shared_ptr<CpuMaterial> cpuMaterial);
		~GpuMaterial() override = default;

		[[nodiscard]] const std::shared_ptr<CpuMaterial>& GetCpuMaterial() const noexcept { return m_CpuMaterial; }
		[[nodiscard]] const ::zzz::core::Guid& GetShaderGuid() const noexcept
		{
			static const ::zzz::core::Guid s_Empty;
			return m_CpuMaterial ? m_CpuMaterial->GetShaderGuid() : s_Empty;
		}

	private:
		std::shared_ptr<CpuMaterial> m_CpuMaterial;
	};
}
