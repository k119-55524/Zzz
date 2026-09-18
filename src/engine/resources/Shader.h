#pragma once

#include <string>
#include "core/resources/ResourceBase.h"

namespace zzz::engine
{
	/**
	 * @class Shader
	 * @brief Ресурс шейдера (CPU/заглушка).
	 */
	class Shader : public ::zzz::core::ResourceBase
	{
	public:
		Shader(const ::zzz::core::Guid& guid, std::string name);
		~Shader() override = default;
	};
}
