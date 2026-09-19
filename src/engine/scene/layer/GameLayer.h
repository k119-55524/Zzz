#pragma once

#include <string>
#include <memory>

#include "engine/scene/layer/ILayer.h"
#include "core/utils/macros/MiscMacros.h"
#include "engine/scene/domain/ObjectDomain.h"
#include "engine/scene/storage/NodeStorage.h"
#include "engine/scene/domain/IEntityDomain.h"
#include "engine/scene/storage/ISpatialStorage.h"

using namespace zzz::core;

namespace zzz::core
{
	class GameObjectData;
}

#include "engine/resources/ResourceTypes.h"

namespace zzz::engine
{
	/**
	 * @class GameLayer
	 * @brief Слой игрового мира сцены (2D/3D), обладающий иерархическим деревом и пространственным индексом.
	 *
	 * @details Инкапсулирует NodeStorage, ISpatialStorage, ObjectDomain и IEntityDomain.
	 * Реализует общий жизненный цикл кадра (Update) и двухпроходный Populate.
	 */
	class GameLayer final : public ILayer
	{
	public:
		GameLayer(
			Guid guid,
			std::string name,
			eLayerType type,
			std::shared_ptr<CoreCpuResourceManager> cpuResourceManager,
			std::shared_ptr<CoreGpuResourceManager> gpuResourceManager,
			std::unique_ptr<ObjectDomain> objectDomain,
			std::unique_ptr<IEntityDomain> entityDomain,
			std::unique_ptr<ISpatialStorage> spatialStorage);
		~GameLayer() override = default;

		Z_NO_COPY_MOVE(GameLayer);

		void Update(float dt) override;
		void Populate(
			const LayerData& layerData,
			const ScriptFactory& scriptFactory,
			std::function<void(std::expected<void, std::string>)> onReady = {}) override;

	private:
		std::shared_ptr<CoreCpuResourceManager>   m_CpuResourceManager;
		std::shared_ptr<CoreGpuResourceManager>   m_GpuResourceManager;

		NodeStorage                           m_NodeStorage;
		std::unique_ptr<ObjectDomain>         m_ObjectDomain;
		std::unique_ptr<IEntityDomain>        m_EntityDomain;
		std::unique_ptr<ISpatialStorage>      m_SpatialStorage;
	};
}
