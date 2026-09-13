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
	 * @struct ObjectRegistration
	 * @brief Результат создания объекта в домене.
	 */
	struct ObjectRegistration
	{
		DomainHandle handle{ kInvalidDomainHandle };
		GameObject*  object{ nullptr };
	};

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

		void Clear();
		void Reserve(size_t capacity);

		[[nodiscard]] GameObject* FindObjectByGuid(const Guid& guid) const noexcept;
		[[nodiscard]] GameObject* FindObjectByName(std::string_view name) const noexcept;
		[[nodiscard]] GameObject* GetObjectByHandle(DomainHandle handle) const noexcept;
		[[nodiscard]] size_t GetObjectCount() const noexcept { return m_Objects.size(); }

		virtual ObjectRegistration CreateObject(const GameObjectData& objData) = 0;

	protected:
		virtual ObjectRegistration CreateObject(const Guid& guid, std::string name, VisualPayload visual = {}) = 0;
		ObjectRegistration RegisterObject(const Guid& guid, std::string name, VisualPayload visual);

	private:
		std::vector<std::unique_ptr<GameObject>> m_Objects;
		std::unordered_map<Guid, DomainHandle> m_ObjectsByGuid;
		std::unordered_map<std::string, std::vector<DomainHandle>> m_ObjectsByName;
	};
}
