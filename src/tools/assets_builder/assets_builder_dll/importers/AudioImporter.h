#pragma once

#include "../IAssetImporter.h"

namespace zzz::builder
{
	class AudioImporter final : public IAssetImporter
	{
	public:
		AudioImporter() = default;

		[[nodiscard]] core::eEngineResourceType GetResourceType() const noexcept override
		{
			return core::eEngineResourceType::AudioClip;
		}

		[[nodiscard]] ImportResult Import(const ImportContext& ctx) override;
	};
}
