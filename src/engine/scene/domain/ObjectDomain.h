#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "core/utils/Guid.h"
#include "core/utils/macros/MiscMacros.h"
#include "engine/scene/gameobject/GameObject.h"

using namespace zzz::core;

namespace zzz::core
{
	class GameObjectData;
}

namespace zzz::engine
{
	class NodeStorage;

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
	 * @class ObjectDomain
	 * @brief Домен классических игровых объектов (GameObject): реестр, быстрый поиск по GUID/имени и создание объектов.
	 */
	class ObjectDomain final
	{
		Z_NO_COPY_MOVE(ObjectDomain);

	public:
		ObjectDomain();
		~ObjectDomain();

		void Clear();

		[[nodiscard]] GameObject* FindObjectByGuid(const Guid& guid) const noexcept;
		[[nodiscard]] GameObject* FindObjectByName(std::string_view name) const noexcept;
		[[nodiscard]] GameObject* GetObjectByHandle(DomainHandle handle) const noexcept;
		[[nodiscard]] size_t GetObjectCount() const noexcept { return m_Objects.size(); }

		ObjectRegistration CreateObject(
			NodeStorage& storage,
			NodeHandle nodeHandle,
			const GameObjectData& objData);

	private:
		ObjectRegistration RegisterObject(
			NodeStorage& storage,
			NodeHandle nodeHandle,
			const Guid& guid,
			std::string name);

		std::vector<std::unique_ptr<GameObject>> m_Objects;
		std::unordered_map<Guid, DomainHandle> m_ObjectsByGuid;
		std::unordered_map<std::string, std::vector<DomainHandle>> m_ObjectsByName;
	};
}
