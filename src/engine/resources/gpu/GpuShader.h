#pragma once

#include <memory>
#include <string>
#include "engine/resources/ResourceBase.h"
#include "engine/resources/ResourceRef.h"
#include "engine/resources/cpu/CpuShader.h"

namespace zzz::engine
{
	/**
	 * @class GpuShader
	 * @brief Ресурс скомпилированного шейдера на GPU.
	 */
	class GpuShader final : public ResourceBase
	{
	public:
		using CpuType = CpuShader;

		GpuShader(
			const ::zzz::core::Guid& guid,
			std::string name,
			ResourceRef<CpuShader> cpuShader);
		~GpuShader() override = default;

		[[nodiscard]] const ResourceRef<CpuShader>& GetCpuShader() const noexcept { return m_CpuShader; }

	private:
		ResourceRef<CpuShader> m_CpuShader;
	};
}
