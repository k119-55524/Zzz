#pragma once

#include "IAssetImporter.h"

namespace zzz::builder
{
	/**
	 * @class PrefabImporter
	 * @brief Заглушка импортёра префабов (.zprefab).
	 * @details Заглушка fail-fast: регистрирует тип ресурса, но возвращает ошибку о нереализованном импорте.
	 */
	class PrefabImporter final : public IAssetImporter
	{
	public:
		PrefabImporter() = default;
		~PrefabImporter() override = default;

		[[nodiscard]] core::eResourceType GetResourceType() const noexcept override
		{
			return core::eResourceType::Prefab;
		}

		[[nodiscard]] ImportResult Import(const ImportContext& ctx) override
		{
			return std::unexpected("Импорт префабов (.zprefab) пока не реализован: '" + ctx.sourceFilePath.string() + "'.");
		}
	};
}
