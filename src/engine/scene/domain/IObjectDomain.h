#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/macros/MiscMacros.h"
#include "engine/scene/visual/VisualTypes.h"
#include "engine/scene/gameobject/GameObject.h"

using namespace zzz::core;

namespace zzz::core
{
	class GameObjectData;
}

namespace zzz::engine
{
	/**
	 * @class IObjectDomain
	 * @brief Базовый класс домена классических объектов: реестр, поиск по GUID/имени и контракт создания объектов.
	 */
	class IObjectDomain
	{
		Z_NO_COPY_MOVE(IObjectDomain);

	public:
		IObjectDomain();
		virtual ~IObjectDomain();

		[[nodiscard]] GameObject* FindObjectByGuid(const Guid& guid) const noexcept;
		[[nodiscard]] GameObject* FindObjectByName(std::string_view name) const noexcept;

		virtual GameObject* CreateObject(const GameObjectData& objData) = 0;
		virtual GameObject* CreateObject(const Guid& guid, std::string name, VisualPayload visual = {}) = 0;

	protected:
		GameObject* RegisterObject(const Guid& guid, std::string name, VisualPayload visual);

	private:
		std::unordered_map<Guid, std::unique_ptr<GameObject>> m_ObjectsByGuid;
		std::unordered_map<std::string, std::vector<GameObject*>> m_ObjectsByName;
	};
}
