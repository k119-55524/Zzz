#pragma once

#include "IAssetImporter.h"

namespace zzz::builder
{
	class ObjImporter final : public IAssetImporter
	{
	public:
		ObjImporter() = default;
		~ObjImporter() override = default;

		[[nodiscard]] core::eEngineResourceType GetResourceType() const noexcept override
		{
			return core::eEngineResourceType::Mesh;
		}

		[[nodiscard]] ImportResult Import(const ImportContext& ctx) override;
	};
}
