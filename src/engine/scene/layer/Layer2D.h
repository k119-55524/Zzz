#pragma once

#include "engine/scene/layer/SceneTreeLayerBase.h"

namespace zzz::engine
{
	/**
	 * @class Layer2D
	 * @brief Слой 2D игрового мира и экранного HUD (спрайты, панели, экранный текст, 2D сценовое дерево).
	 */
	class Layer2D final : public SceneTreeLayerBase
	{
		Z_NO_COPY_MOVE(Layer2D);

	public:
		Layer2D(std::string name, std::shared_ptr<ResourceManager> resourceManager = nullptr);
		~Layer2D() override = default;

		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::Layer2D; }
	};
}
