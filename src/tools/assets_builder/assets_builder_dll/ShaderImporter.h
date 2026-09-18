#pragma once

#include "IAssetImporter.h"

namespace zzz::builder
{
	/**
	 * @class ShaderImporter
	 * @brief Импортёр шейдеров (.zshaders).
	 */
	class ShaderImporter final : public IAssetImporter
	{
	public:
		ShaderImporter() = default;
		~ShaderImporter() override = default;

		[[nodiscard]] core::eResourceType GetResourceType() const noexcept override
		{
			return core::eResourceType::Shader;
		}

		[[nodiscard]] ImportResult Import(const ImportContext& ctx) override;
	};
}
