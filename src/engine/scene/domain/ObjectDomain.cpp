
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

#include "ObjectDomain.h"

namespace zzz::engine
{
	ObjectDomain::ObjectDomain() = default;
	ObjectDomain::~ObjectDomain() = default;

	GameObject* ObjectDomain::CreateObject(const Guid& guid, std::string name)
	{
		ensure(!m_ObjectsByGuid.contains(guid), "ObjectDomain::CreateObject: объект с GUID '{}' уже зарегистрирован в домене.", guid.ToString());

		const std::string nameCopy = name;
		auto obj = safe_make_unique<GameObject>(guid, std::move(name));
		GameObject* rawPtr = obj.get();
		m_ObjectsByGuid[guid] = std::move(obj);
		m_ObjectsByName[nameCopy].push_back(rawPtr);

		return rawPtr;
	}

	GameObject* ObjectDomain::FindObjectByGuid(const Guid& guid) const noexcept
	{
		auto it = m_ObjectsByGuid.find(guid);
		return it != m_ObjectsByGuid.end() ? it->second.get() : nullptr;
	}

	GameObject* ObjectDomain::FindObjectByName(std::string_view name) const noexcept
	{
		auto it = m_ObjectsByName.find(std::string(name));
		if (it != m_ObjectsByName.end() && !it->second.empty())
			return it->second.front();

		return nullptr;
	}

	void ObjectDomain::Update(float /*dt*/)
	{
	}

	void ObjectDomain::Clear()
	{
		m_ObjectsByName.clear();
		m_ObjectsByGuid.clear();
	}
}
