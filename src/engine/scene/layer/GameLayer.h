#pragma once

#include <string>
#include <memory>

#include "engine/scene/layer/ILayer.h"
#include "core/utils/macros/MiscMacros.h"
#include "engine/scene/domain/IObjectDomain.h"
#include "engine/scene/domain/IEntityDomain.h"
#include "engine/scene/storage/ISpatialStorage.h"
#include "engine/scene/storage/NodeStorage.h"

using namespace zzz::core;

namespace zzz::core
{
	class GameObjectData;
}

namespace zzz::engine
{
	class ResourceManager;

	/**
	 * @class GameLayer
	 * @brief Слой игрового мира сцены (2D/3D), обладающий иерархическим деревом и пространственным индексом.
	 *
	 * @details Инкапсулирует NodeStorage, ISpatialStorage, IObjectDomain и IEntityDomain.
	 * Реализует общий жизненный цикл кадра (BeginFrame, Update) и двухпроходный Populate.
	 */
	class GameLayer final : public ILayer
	{
	public:
		GameLayer(
			std::string name,
			eLayerType type,
			std::shared_ptr<ResourceManager> resourceManager,
			std::unique_ptr<IObjectDomain> objectDomain,
			std::unique_ptr<IEntityDomain> entityDomain,
			std::unique_ptr<ISpatialStorage> spatialStorage);
		~GameLayer() override = default;

		Z_NO_COPY_MOVE(GameLayer);

		void BeginFrame() override;
		void Update(float dt) override;
		void Populate(
			const LayerData& layerData,
			const ScriptFactory& scriptFactory) override;

	private:
		void OnUpdateDomains(float dt);
		void OnUpdateSpatial();

		void PopulateGameObject(zU32 nodeIndex, const GameObjectData& objData, const ScriptFactory& scriptFactory);
		void PopulateEntity(zU32 nodeIndex, const GameObjectData& objData);

		std::shared_ptr<ResourceManager> m_ResourceManager;

		std::unique_ptr<IObjectDomain>   m_ObjectDomain;
		std::unique_ptr<IEntityDomain>   m_EntityDomain;
		std::unique_ptr<ISpatialStorage> m_SpatialStorage;
		NodeStorage                      m_NodeStorage;
	};
}
