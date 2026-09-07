#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "core/utils/Guid.h"
#include "core/utils/Defines.h"

namespace zzz::engine
{
	/**
	 * @struct EntityStub
	 * @brief Заглушка представления сущности для сквозной цепочки domain == Entity.
	 */
	struct EntityStub
	{
		::zzz::core::Guid guid;
		std::string name;
	};

	/**
	 * @class EntityWorld
	 * @brief Легковесная заглушка мира ECS-сущностей для сквозной цепочки domain == Entity.
	 */
	class EntityWorld final
	{
	public:
		EntityWorld() = default;
		~EntityWorld() = default;

		Z_NO_COPY_MOVE(EntityWorld);

		void CreateEntity(const ::zzz::core::Guid& guid, std::string_view name);
		void DestroyEntity(const ::zzz::core::Guid& guid);
		void Update(float dt);

		[[nodiscard]] size_t GetEntityCount() const noexcept { return m_Entities.size(); }
		[[nodiscard]] const std::vector<EntityStub>& GetEntities() const noexcept { return m_Entities; }

	private:
		std::vector<EntityStub> m_Entities;
	};
}
