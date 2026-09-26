#pragma once

#include <expected>
#include <span>
#include <string>
#include <vector>

#include "../IAssetImporter.h"
#include "TextureBuilder.h"

namespace zzz::builder
{
	class TextureImporter final : public IAssetImporter
	{
	public:
		TextureImporter() = default;

		[[nodiscard]] core::eEngineResourceType GetResourceType() const noexcept override
		{
			return core::eEngineResourceType::Texture2D;
		}

		[[nodiscard]] ImportResult Import(const ImportContext& ctx) override;

		/**
		 * @brief Быстрое извлечение информации о текстуре без полного декодирования.
		 */
		[[nodiscard]] std::expected<zzz::texture::ImageInfo, std::string> Probe(std::span<const zU8> fileBytes) const;

	private:
		zzz::texture::TextureBuilder m_Builder;
	};
}
