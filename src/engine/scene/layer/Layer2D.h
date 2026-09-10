#pragma once

#include "engine/scene/layer/SpatialLayer.h"

namespace zzz::engine
{
	/**
	 * @class Layer2D
	 * @brief Слой 2D игрового мира и экранного HUD (спрайты, панели, экранный текст, 2D сценовое дерево).
	 */
	class Layer2D final : public SpatialLayer
	{
		Z_NO_COPY_MOVE(Layer2D);

	public:
		Layer2D(
			std::string name,
			std::shared_ptr<ResourceManager> resourceManager,
			std::unique_ptr<IObjectDomain> objectDomain,
			std::unique_ptr<IEntityDomain> entityDomain,
			std::unique_ptr<ISpatialStorage> spatialStorage);
		~Layer2D() override = default;
	};
}
