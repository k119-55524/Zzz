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
		explicit EntityDomain(::zzz::core::eLayerType layerType = ::zzz::core::eLayerType::Layer3D);
		~EntityDomain() override;

		Z_NO_COPY_MOVE(EntityDomain);

		[[nodiscard]] ::zzz::core::eLayerType GetLayerType() const noexcept { return m_LayerType; }

		// --- ILayerDomain ---
		void Update(float dt) override;
		void Clear() override;

		// --- IEntityDomain ---
		void CreateEntity(const ::zzz::core::Guid& guid, std::string_view name) override;
		void DestroyEntity(const ::zzz::core::Guid& guid) override;

		[[nodiscard]] size_t GetEntityCount() const noexcept;
		[[nodiscard]] const std::vector<EntityStub>& GetEntities() const noexcept;

	private:
		::zzz::core::eLayerType m_LayerType{ ::zzz::core::eLayerType::Layer3D };
		EntityWorld m_World;
	};
}
