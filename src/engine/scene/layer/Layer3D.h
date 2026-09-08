#pragma once

#include "engine/scene/layer/SceneTreeLayerBase.h"

namespace zzz::engine
{
	/**
	 * @class Layer3D
	 * @brief Слой 3D игрового мира (GameObject, сценовое дерево, меши, материалы, освещение).
	 */
	class Layer3D final : public SceneTreeLayerBase
	{
		Z_NO_COPY_MOVE(Layer3D);

	public:
		Layer3D(std::string name, std::shared_ptr<ResourceManager> resourceManager);
		~Layer3D() override = default;

		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::Layer3D; }
	};
}
