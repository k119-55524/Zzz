#pragma once

#include <memory>
#include <string>
#include "core/resources/ResourceBase.h"
#include "engine/resources/cpu/CpuShader.h"

namespace zzz::engine
{
	/**
	 * @class GpuShader
	 * @brief Ресурс скомпилированного шейдера на GPU.
	 */
	class GpuShader final : public ::zzz::core::ResourceBase
	{
	public:
		GpuShader(
			const ::zzz::core::Guid& guid,
			std::string name,
			std::shared_ptr<CpuShader> cpuShader);
		~GpuShader() override = default;

		[[nodiscard]] const std::shared_ptr<CpuShader>& GetCpuShader() const noexcept { return m_CpuShader; }

	private:
		std::shared_ptr<CpuShader> m_CpuShader;
	};
}
