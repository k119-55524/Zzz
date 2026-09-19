#pragma once

#include <string>
#include "engine/resources/ResourceBase.h"

namespace zzz::engine
{
	/**
	 * @class CpuShader
	 * @brief Ресурс шейдера в оперативной памяти (CPU).
	 */
	class CpuShader final : public ResourceBase
	{
	public:
		CpuShader(const ::zzz::core::Guid& guid, std::string name);
		~CpuShader() override = default;
	};
}
