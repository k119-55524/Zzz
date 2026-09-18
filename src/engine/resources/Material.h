#pragma once

#include <string>
#include "core/resources/ResourceBase.h"

namespace zzz::engine
{
	/**
	 * @class Material
	 * @brief Ресурс материала (CPU/заглушка).
	 */
	class Material : public ::zzz::core::ResourceBase
	{
	public:
		Material(const ::zzz::core::Guid& guid, std::string name, ::zzz::core::Guid shaderGuid = {});
		~Material() override = default;

		[[nodiscard]] const ::zzz::core::Guid& GetShaderGuid() const noexcept { return m_ShaderGuid; }

	private:
		::zzz::core::Guid m_ShaderGuid;
	};
}
