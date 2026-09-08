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

namespace zzz::engine
{
	class ResourceManager;

	/**
	 * @class SceneTreeLayerBase
	 * @brief Базовый класс для слоёв сцены, обладающих иерархическим деревом и пространственным индексом.
	 *
	 * @details Инкапсулирует двухбуферный SceneTreeContainer, ISpatialStorage, IObjectDomain и IEntityDomain.
	 * Реализует общий жизненный цикл кадра (BeginFrame, Update, ApplyHandoverBarrier) и двухпроходный Populate.
	 */
	class SceneTreeLayerBase : public ILayer
	{
	public:
		SceneTreeLayerBase(
			std::string name,
			std::shared_ptr<ResourceManager> resourceManager,
			std::unique_ptr<IObjectDomain> objectDomain,
			std::unique_ptr<IEntityDomain> entityDomain,
			std::unique_ptr<ISpatialStorage> spatialStorage);
		~SceneTreeLayerBase() override = default;

		Z_NO_COPY_MOVE(SceneTreeLayerBase);

		[[nodiscard]] const std::string& GetName() const noexcept override { return m_Name; }
		[[nodiscard]] bool IsVisible() const noexcept override { return m_IsVisible; }
		void SetVisible(bool visible) noexcept override { m_IsVisible = visible; }

		void BeginFrame() override;
		void Update(float dt) override;
		void ApplyHandoverBarrier() override;
		void Populate(
			const LayerData& layerData,
			const ScriptFactory& scriptFactory) override;

		[[nodiscard]] SceneTreeContainer& GetTreeContainer() noexcept { return m_TreeContainer; }
		[[nodiscard]] const SceneTreeContainer& GetTreeContainer() const noexcept { return m_TreeContainer; }

		[[nodiscard]] ISpatialStorage& GetSpatialStorage() noexcept { return *m_SpatialStorage; }
		[[nodiscard]] IObjectDomain& GetObjectDomain() noexcept { return *m_ObjectDomain; }
		[[nodiscard]] IEntityDomain& GetEntityDomain() noexcept { return *m_EntityDomain; }

	protected:
		virtual void OnUpdateDomains(float dt);
		virtual void OnUpdateSpatial();

		std::string m_Name;
		bool m_IsVisible;
		std::shared_ptr<ResourceManager> m_ResourceManager;

		std::unique_ptr<IObjectDomain> m_ObjectDomain;
		std::unique_ptr<IEntityDomain> m_EntityDomain;

		std::unique_ptr<ISpatialStorage> m_SpatialStorage;
		SceneTreeContainer m_TreeContainer;
	};
}
