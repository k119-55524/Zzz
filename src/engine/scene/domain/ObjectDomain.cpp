
#include "core/utils/MemoryUtils.h"
#include "core/utils/Ensure.h"

#include "ObjectDomain.h"

namespace zzz::engine
{
	ObjectDomain::ObjectDomain() = default;
	ObjectDomain::~ObjectDomain() = default;

	GameObject* ObjectDomain::AddObject(const Guid& guid, std::string name)
	{
		ensure(!m_Objects.contains(guid), "ObjectDomain::AddObject: объект с GUID '{}' уже зарегистрирован в домене.", guid.ToString());

		auto obj = safe_make_unique<GameObject>(guid, std::move(name));
		GameObject* rawPtr = obj.get();
		m_Objects[guid] = std::move(obj);

		return rawPtr;
	}

	GameObject* ObjectDomain::FindObjectByGuid(const Guid& guid) const noexcept
	{
		auto it = m_Objects.find(guid);
		return it != m_Objects.end() ? it->second.get() : nullptr;
	}

	void ObjectDomain::GetAllObjects(std::vector<GameObject*>& outObjects) const
	{
		outObjects.clear();
		outObjects.reserve(m_Objects.size());
		for (const auto& [guid, obj] : m_Objects)
		{
			if (obj != nullptr)
			{
				outObjects.push_back(obj.get());
			}
		}
	}

	void ObjectDomain::Update(float /*dt*/)
	{
	}

	void ObjectDomain::Clear()
	{
		m_Objects.clear();
	}
}
