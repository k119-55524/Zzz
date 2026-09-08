#include "engine/scene/domain/ObjectDomain.h"
#include "core/utils/MemoryUtils.h"

namespace zzz::engine
{
	ObjectDomain::ObjectDomain() = default;
	ObjectDomain::~ObjectDomain() = default;

	::zzz::GameObject* ObjectDomain::CreateObject(const ::zzz::core::Guid& guid, std::string name)
	{
		auto obj = ::zzz::core::safe_make_unique<::zzz::GameObject>(guid, std::move(name));
		::zzz::GameObject* rawPtr = obj.get();

		m_AllocatedObjects.emplace(rawPtr, std::move(obj));
		if (guid != ::zzz::core::Guid{})
		{
			m_GuidToObject[guid] = rawPtr;
		}

		const ::zzz::core::SlotHandle handle = m_ActiveObjects.Emplace(rawPtr);
		rawPtr->SetWorldHandle(handle);

		return rawPtr;
	}

	void ObjectDomain::DestroyObject(::zzz::GameObject* obj)
	{
		if (obj == nullptr)
		{
			return;
		}

		if (obj->GetGuid() != ::zzz::core::Guid{})
		{
			m_GuidToObject.erase(obj->GetGuid());
		}

		const ::zzz::core::SlotHandle handle = obj->GetWorldHandle();
		if (handle.IsValid())
		{
			m_ActiveObjects.Remove(handle);
			obj->SetWorldHandle(::zzz::core::SlotHandle{});
		}

		m_AllocatedObjects.erase(obj);
	}

	::zzz::GameObject* ObjectDomain::FindObjectByGuid(const ::zzz::core::Guid& guid) const noexcept
	{
		auto it = m_GuidToObject.find(guid);
		return it != m_GuidToObject.end() ? it->second : nullptr;
	}

	void ObjectDomain::GetAllObjects(std::vector<::zzz::GameObject*>& outObjects) const
	{
		outObjects.clear();
		outObjects.reserve(m_ActiveObjects.Size());
		for (::zzz::GameObject* obj : m_ActiveObjects.GetDenseSpan())
		{
			if (obj != nullptr)
			{
				outObjects.push_back(obj);
			}
		}
	}

	size_t ObjectDomain::GetObjectCount() const noexcept
	{
		return m_ActiveObjects.Size();
	}

	std::span<::zzz::GameObject* const> ObjectDomain::GetObjects() const noexcept
	{
		return m_ActiveObjects.GetDenseSpan();
	}

	void ObjectDomain::Update(float /*dt*/)
	{
		for (::zzz::GameObject* obj : m_ActiveObjects.GetDenseSpan())
		{
			if (obj != nullptr && obj->IsActive())
			{
				obj->DecrementRenderFrames();
			}
		}
	}

	void ObjectDomain::Clear()
	{
		m_ActiveObjects.Clear();
		m_GuidToObject.clear();
		m_AllocatedObjects.clear();
	}
}
