#pragma once

#include "core/utils/Guid.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/enums/eLayerType.h"
#include "engine/scene/domain/IEntityDomain.h"
#include "engine/scene/entity/EntityWorld.h"

namespace zzz::engine
{
	/**
	 * @class EntityDomain
	 * @brief Реализация IEntityDomain для управления легковесными сущностями (ECS).
	 */
	class EntityDomain final : public IEntityDomain
	{
	public:
		EntityDomain();
		~EntityDomain() override;

		Z_NO_COPY_MOVE(EntityDomain);

		void Update(float dt) override;
		void Clear() override;

		DomainHandle CreateEntity(const ::zzz::core::Guid& guid, std::string_view name) override;
		DomainHandle CreateEntity(
			NodeHandle nodeHandle,
			const ::zzz::core::GameObjectData& objData,
			const ::zzz::core::ScriptFactory& scriptFactory,
			class ResourceManager& resourceManager) override;
		void DestroyEntity(const ::zzz::core::Guid& guid) override;

		[[nodiscard]] size_t GetEntityCount() const noexcept;
		[[nodiscard]] const std::vector<EntityStub>& GetEntities() const noexcept;

	private:
		EntityWorld m_World;
	};
}
