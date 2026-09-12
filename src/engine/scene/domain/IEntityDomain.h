#pragma once

#include <string>
#include <string_view>
#include <cstdint>

#include "core/utils/Guid.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/utils/Defines.h"

namespace zzz::core
{
	class GameObjectData;
	class ScriptFactory;
}

namespace zzz::engine
{
	/**
	 * @class IEntityDomain
	 * @brief Контракт домена управления легковесными сущностями (ECS).
	 */
	class IEntityDomain
	{
		Z_NO_COPY_MOVE(IEntityDomain);

	public:
		IEntityDomain() = default;
		virtual ~IEntityDomain() = default;

		virtual void Update(float dt) = 0;
		virtual void CreateEntity(const ::zzz::core::Guid& guid, std::string_view name) = 0;
		virtual void CreateEntity(
			zU32 nodeIndex,
			const ::zzz::core::GameObjectData& objData,
			const ::zzz::core::ScriptFactory& scriptFactory,
			class ResourceManager& resourceManager) = 0;
		virtual void DestroyEntity(const ::zzz::core::Guid& guid) = 0;
	};
}
