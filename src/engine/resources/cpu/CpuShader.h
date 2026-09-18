#pragma once

#include <string>
#include "core/resources/ResourceBase.h"

namespace zzz::engine
{
	/**
	 * @class CpuShader
	 * @brief Ресурс шейдера в оперативной памяти (CPU).
	 */
	class CpuShader final : public ::zzz::core::ResourceBase
	{
	public:
		CpuShader(const ::zzz::core::Guid& guid, std::string name);
		~CpuShader() override = default;
	};
}
