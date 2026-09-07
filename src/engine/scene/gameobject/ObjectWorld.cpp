#include "engine/scene/gameobject/ObjectWorld.h"
#include "engine/scene/storage/ISpatialStorage.h"
#include "core/utils/MemoryUtils.h"
#include <algorithm>

namespace zzz
{
	ObjectWorld::ObjectWorld() = default;
	ObjectWorld::~ObjectWorld() = default;

	GameObject* ObjectWorld::CreateObject(std::string name)
	{
		return CreateObject(::zzz::core::Guid{}, std::move(name));
	}

	GameObject* ObjectWorld::CreateObject(const ::zzz::core::Guid& guid, std::string name)
	{
		auto obj = ::zzz::core::safe_make_unique<GameObject>(guid, std::move(name));
		GameObject* rawPtr = obj.get();

		// Стабильное владение памятью: гарантированная O(1) вставка
		m_AllocatedObjects.emplace(rawPtr, std::move(obj));

		// Добавление в плотный пул SlotMap
		const ::zzz::core::SlotHandle handle = m_ActiveObjects.Emplace(rawPtr);
		rawPtr->SetWorldHandle(handle);

		// Регистрация в пространственном хранилище слоя
		if (m_Storage != nullptr)
		{
			const uint32_t spHandle = m_Storage->Insert(reinterpret_cast<uint64_t>(rawPtr));
			rawPtr->SetSpatialHandle(spHandle);
		}

		return rawPtr;
	}

	void ObjectWorld::DestroyObject(GameObject* obj)
	{
		if (obj == nullptr)
		{
			return;
		}

		// Рекурсивно удаляем дочерние объекты
		while (obj->GetChildCount() > 0)
		{
			DestroyObject(obj->GetChild(0));
		}

		// Отвязываем от родителя
		if (obj->GetParent() != nullptr)
		{
			obj->SetParent(nullptr, false);
		}

		// Удаление из пространственного хранилища слоя
		if (m_Storage != nullptr && obj->GetSpatialHandle() != 0xFFFFFFFF)
		{
			m_Storage->Remove(obj->GetSpatialHandle());
			obj->SetSpatialHandle(0xFFFFFFFF);
		}

		// O(1) удаление из плотного массива через SlotHandle
		const ::zzz::core::SlotHandle handle = obj->GetWorldHandle();
		if (handle.IsValid())
		{
			m_ActiveObjects.Remove(handle);
			obj->SetWorldHandle(::zzz::core::SlotHandle{});
		}

		// O(1) удаление из владеющего хэш-контейнера уникальных указателей
		m_AllocatedObjects.erase(obj);
	}

	void ObjectWorld::Clear()
	{
		if (m_Storage != nullptr)
		{
			m_Storage->Clear();
		}
		m_ActiveObjects.Clear();
		m_AllocatedObjects.clear();
	}

	size_t ObjectWorld::GetObjectCount() const noexcept
	{
		return m_ActiveObjects.Size();
	}

	std::span<GameObject* const> ObjectWorld::GetObjects() const noexcept
	{
		return m_ActiveObjects.GetDenseSpan();
	}

	void ObjectWorld::Update(float /*dt*/)
	{
		auto activeSpan = m_ActiveObjects.GetDenseSpan();
		for (GameObject* obj : activeSpan)
		{
			if (obj != nullptr && obj->IsActive())
			{
				// Инвалидация кадров полёта для GPU
				obj->DecrementRenderFrames();
			}
		}
	}
}
