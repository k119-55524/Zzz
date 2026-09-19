#pragma once

#include <string>
#include <string_view>
#include <cstdint>

#include "core/utils/Guid.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/utils/Defines.h"
#include "engine/scene/storage/NodeTypes.h"

namespace zzz::core
{
	class GameObjectData;
	class ScriptFactory;
}

namespace zzz::engine
{
	class CpuResourceManager;

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

		virtual void Clear() = 0;

		virtual void Update(float dt) = 0;
		virtual DomainHandle CreateEntity(const ::zzz::core::Guid& guid, std::string_view name) = 0;
		virtual DomainHandle CreateEntity(
			NodeHandle nodeHandle,
			const ::zzz::core::GameObjectData& objData,
			const ::zzz::core::ScriptFactory& scriptFactory,
			CpuResourceManager& resourceManager) = 0;
		virtual void DestroyEntity(const ::zzz::core::Guid& guid) = 0;
	};
}
