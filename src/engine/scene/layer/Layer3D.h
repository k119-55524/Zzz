#pragma once

#include <memory>

#include "engine/scene/layer/ILayer.h"
#include "engine/scene/ObjectWorld.h"
#include "engine/scene/EntityWorld.h"
#include "engine/scene/storage/DefaultSceneStorage.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @class Layer3D
	 * @brief Слой 3D игрового мира (GameObject, Transform, меши, материалы, освещение).
	 */
	class Layer3D final : public ILayer
	{
	public:
		Layer3D(std::string name, std::shared_ptr<::zzz::engine::ResourceManager> resourceManager);
		~Layer3D() override = default;

		Z_NO_COPY_MOVE(Layer3D);

		[[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
		[[nodiscard]] eLayerType GetType() const noexcept override { return eLayerType::Layer3D; }

		[[nodiscard]] bool IsVisible() const noexcept override { return m_IsVisible; }
		void SetVisible(bool visible) noexcept override { m_IsVisible = visible; }

		void Update(float dt) override;

		void Populate(const LayerData& layerData, const ScriptFactory& scriptFactory) override;

	private:
		std::string m_Name;
		bool m_IsVisible;

		std::unique_ptr<ISceneStorage> m_Storage;
		ObjectWorld m_ObjectWorld;
		EntityWorld m_EntityWorld;
		std::shared_ptr<ResourceManager> m_ResourceManager;
	};
}
