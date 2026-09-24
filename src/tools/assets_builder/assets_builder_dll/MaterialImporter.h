#pragma once

#include "IAssetImporter.h"

namespace zzz::builder
{
	class MaterialImporter final : public IAssetImporter
	{
	public:
		MaterialImporter() = default;
		~MaterialImporter() override = default;

		[[nodiscard]] core::eEngineResourceType GetResourceType() const noexcept override
		{
			return core::eEngineResourceType::Material;
		}

		[[nodiscard]] ImportResult Import(const ImportContext& ctx) override;
	};
}
