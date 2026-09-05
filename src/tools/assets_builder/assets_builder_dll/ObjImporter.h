#pragma once

#include "IAssetImporter.h"

namespace zzz::builder
{
	class ObjImporter final : public IAssetImporter
	{
	public:
		ObjImporter() = default;
		~ObjImporter() override = default;

		[[nodiscard]] ImportResult Import(const ImportContext& ctx) override;
	};
}
