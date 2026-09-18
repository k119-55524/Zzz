#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"
#include "core/io/package/GameObjectData.h"

#include "ObjectDomain.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	ObjectDomain::ObjectDomain()
		: m_Objects()
		, m_ObjectsByGuid()
		, m_ObjectsByName()
	{
	}

	ObjectDomain::~ObjectDomain() = default;

	void ObjectDomain::Clear()
	{
		m_Objects.clear();
		m_ObjectsByGuid.clear();
		m_ObjectsByName.clear();
	}

	ObjectRegistration ObjectDomain::CreateObject(
		NodeStorage& storage,
		NodeHandle nodeHandle,
		const GameObjectData& objData)
	{
		return RegisterObject(storage, nodeHandle, objData.GetGuid(), objData.GetName());
	}

	ObjectRegistration ObjectDomain::RegisterObject(
		NodeStorage& storage,
		NodeHandle nodeHandle,
		const Guid& guid,
		std::string name)
	{
		ensure(!m_ObjectsByGuid.contains(guid), "ObjectDomain::RegisterObject: объект с GUID '{}' уже зарегистрирован в домене.", guid.ToString());

		const DomainHandle handle = static_cast<DomainHandle>(m_Objects.size());
		const std::string nameCopy = name;
		auto obj = safe_make_shared<GameObject>(guid, std::move(name), storage, nodeHandle);
		GameObject* rawPtr = obj.get();
		m_Objects.push_back(std::move(obj));
		m_ObjectsByGuid[guid] = handle;
		m_ObjectsByName[nameCopy].push_back(handle);

		return ObjectRegistration{
			.handle = handle,
			.object = rawPtr
		};
	}

	GameObject* ObjectDomain::FindObjectByGuid(const Guid& guid) const noexcept
	{
		auto it = m_ObjectsByGuid.find(guid);
		return it != m_ObjectsByGuid.end() ? GetObjectByHandle(it->second) : nullptr;
	}

	GameObject* ObjectDomain::FindObjectByName(std::string_view name) const noexcept
	{
		auto it = m_ObjectsByName.find(std::string(name));
		if (it != m_ObjectsByName.end() && !it->second.empty())
			return GetObjectByHandle(it->second.front());

		return nullptr;
	}

	GameObject* ObjectDomain::GetObjectByHandle(DomainHandle handle) const noexcept
	{
		if (handle < m_Objects.size())
		{
			return m_Objects[handle].get();
		}
		return nullptr;
	}
}
