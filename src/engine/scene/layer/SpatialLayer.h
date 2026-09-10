#pragma once

#include <string>
#include <memory>

#include "engine/scene/layer/ILayer.h"
#include "core/utils/macros/MiscMacros.h"
#include "engine/scene/domain/IObjectDomain.h"
#include "engine/scene/domain/IEntityDomain.h"
#include "engine/scene/storage/ISpatialStorage.h"
#include "engine/scene/storage/SceneTreeContainer.h"

using namespace zzz::core;

namespace zzz::core
{
	class GameObjectData;
}

namespace zzz::engine
{
	class ResourceManager;

	/**
	 * @class SpatialLayer
	 * @brief Базовый класс для слоёв сцены, обладающих иерархическим деревом и пространственным индексом.
	 *
	 * @details Инкапсулирует двухбуферный SceneTreeContainer, ISpatialStorage, IObjectDomain и IEntityDomain.
	 * Реализует общий жизненный цикл кадра (BeginFrame, Update, ApplyHandoverBarrier) и двухпроходный Populate.
	 */
	class SpatialLayer : public ILayer
	{
	public:
		SpatialLayer(
			std::string name,
			eLayerType type,
			std::shared_ptr<ResourceManager> resourceManager,
			std::unique_ptr<IObjectDomain> objectDomain,
			std::unique_ptr<IEntityDomain> entityDomain,
			std::unique_ptr<ISpatialStorage> spatialStorage);
		~SpatialLayer() override = default;

		Z_NO_COPY_MOVE(SpatialLayer);

		void BeginFrame() override;
		void Update(float dt) override;
		void ApplyHandoverBarrier() override;
		void Populate(
			const LayerData& layerData,
			const ScriptFactory& scriptFactory) override;

		[[nodiscard]] SceneTreeContainer& GetTreeContainer() noexcept { return m_TreeContainer; }
		[[nodiscard]] const SceneTreeContainer& GetTreeContainer() const noexcept { return m_TreeContainer; }

	protected:
		virtual void OnUpdateDomains(float dt);
		virtual void OnUpdateSpatial();

		std::shared_ptr<ResourceManager> m_ResourceManager;

		std::unique_ptr<IObjectDomain>   m_ObjectDomain;
		std::unique_ptr<IEntityDomain>   m_EntityDomain;
		std::unique_ptr<ISpatialStorage> m_SpatialStorage;
		SceneTreeContainer               m_TreeContainer;

	private:
		[[nodiscard]] NodeHandle PopulateGameObject(const GameObjectData& objData, const ScriptFactory& scriptFactory);
		[[nodiscard]] NodeHandle PopulateEntity(const GameObjectData& objData);
	};
}
