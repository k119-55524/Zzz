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
		Material(const ::zzz::core::Guid& guid, std::string name);
		~Material() override = default;
	};
}
