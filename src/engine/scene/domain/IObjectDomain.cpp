#include "IObjectDomain.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	IObjectDomain::IObjectDomain()
		: m_Objects()
		, m_ObjectsByGuid()
		, m_ObjectsByName()
	{
	}

	IObjectDomain::~IObjectDomain() = default;

	void IObjectDomain::Clear()
	{
		m_Objects.clear();
		m_ObjectsByGuid.clear();
		m_ObjectsByName.clear();
	}

	void IObjectDomain::Reserve(size_t capacity)
	{
		m_Objects.reserve(capacity);
		m_ObjectsByGuid.reserve(capacity);
		m_ObjectsByName.reserve(capacity);
	}

	ObjectRegistration IObjectDomain::RegisterObject(const Guid& guid, std::string name, VisualPayload visual)
	{
		ensure(!m_ObjectsByGuid.contains(guid), "IObjectDomain::RegisterObject: объект с GUID '{}' уже зарегистрирован в домене.", guid.ToString());

		const DomainHandle handle = static_cast<DomainHandle>(m_Objects.size());
		const std::string nameCopy = name;
		auto obj = safe_make_unique<GameObject>(guid, std::move(name), std::move(visual));
		GameObject* rawPtr = obj.get();
		m_Objects.push_back(std::move(obj));
		m_ObjectsByGuid[guid] = handle;
		m_ObjectsByName[nameCopy].push_back(handle);

		return ObjectRegistration{
			.handle = handle,
			.object = rawPtr
		};
	}

	GameObject* IObjectDomain::FindObjectByGuid(const Guid& guid) const noexcept
	{
		auto it = m_ObjectsByGuid.find(guid);
		return it != m_ObjectsByGuid.end() ? GetObjectByHandle(it->second) : nullptr;
	}

	GameObject* IObjectDomain::FindObjectByName(std::string_view name) const noexcept
	{
		auto it = m_ObjectsByName.find(std::string(name));
		if (it != m_ObjectsByName.end() && !it->second.empty())
			return GetObjectByHandle(it->second.front());

		return nullptr;
	}

	GameObject* IObjectDomain::GetObjectByHandle(DomainHandle handle) const noexcept
	{
		if (handle < m_Objects.size())
		{
			return m_Objects[handle].get();
		}
		return nullptr;
	}
}
