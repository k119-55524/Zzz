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

		[[nodiscard]] core::eEngineResourceType GetResourceType() const noexcept override
		{
			return core::eEngineResourceType::Shader;
		}

		[[nodiscard]] ImportResult Import(const ImportContext& ctx) override;
	};
}
