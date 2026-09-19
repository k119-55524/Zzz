#pragma once

#include <string>
#include "engine/resources/ResourceBase.h"
#include "core/utils/Guid.h"

namespace zzz::engine
{
	/**
	 * @class CpuMaterial
	 * @brief Ресурс описания материала в оперативной памяти (CPU).
	 */
	class CpuMaterial final : public ResourceBase
	{
	public:
		CpuMaterial(const ::zzz::core::Guid& guid, std::string name, ::zzz::core::Guid shaderGuid = {});
		~CpuMaterial() override = default;

		[[nodiscard]] const ::zzz::core::Guid& GetShaderGuid() const noexcept { return m_ShaderGuid; }

	private:
		::zzz::core::Guid m_ShaderGuid;
	};
}
