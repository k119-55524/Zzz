#include "IObjectDomain.h"
#include "core/utils/Ensure.h"
#include "core/utils/MemoryUtils.h"

using namespace zzz::core;

Z_SET_LOG_CATEGORY(zzz::core::Scene);

namespace zzz::engine
{
	IObjectDomain::IObjectDomain()
		: m_ObjectsByGuid{}
		, m_ObjectsByName{}
	{
	}

	IObjectDomain::~IObjectDomain() = default;

	GameObject* IObjectDomain::RegisterObject(const Guid& guid, std::string name, VisualPayload visual)
	{
		ensure(!m_ObjectsByGuid.contains(guid), "IObjectDomain::RegisterObject: объект с GUID '{}' уже зарегистрирован в домене.", guid.ToString());

		const std::string nameCopy = name;
		auto obj = safe_make_unique<GameObject>(guid, std::move(name), std::move(visual));
		GameObject* rawPtr = obj.get();
		m_ObjectsByGuid[guid] = std::move(obj);
		m_ObjectsByName[nameCopy].push_back(rawPtr);

		return rawPtr;
	}

	GameObject* IObjectDomain::FindObjectByGuid(const Guid& guid) const noexcept
	{
		auto it = m_ObjectsByGuid.find(guid);
		return it != m_ObjectsByGuid.end() ? it->second.get() : nullptr;
	}

	GameObject* IObjectDomain::FindObjectByName(std::string_view name) const noexcept
	{
		auto it = m_ObjectsByName.find(std::string(name));
		if (it != m_ObjectsByName.end() && !it->second.empty())
			return it->second.front();

		return nullptr;
	}
}
