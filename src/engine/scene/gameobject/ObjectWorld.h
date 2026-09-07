#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <span>
#include "core/utils/Guid.h"
#include "core/utils/Defines.h"
#include "core/templates/SlotMap.h"
#include "engine/scene/gameobject/GameObject.h"

namespace zzz::engine
{
	class ISpatialStorage;
}

namespace zzz
{
	using ::zzz::engine::ISpatialStorage;

	/**
	 * @class ObjectWorld
	 * @brief Владелец жизненного цикла и логики игровых объектов сцены (GameObject).
	 *
	 * @details Реализует стабильное владение памятью через уникальные указатели
	 * и быстрый непрерывный доступ через плотный массив указателей SlotMap<GameObject*>.
	 */
	class ObjectWorld final
	{
	public:
		ObjectWorld();
		~ObjectWorld();

		Z_NO_COPY_MOVE(ObjectWorld);

		// --- Создание и удаление объектов ---
		GameObject* CreateObject(std::string name = "GameObject");
		GameObject* CreateObject(const ::zzz::core::Guid& guid, std::string name);
		void DestroyObject(GameObject* obj);
		void Clear();

		// --- Доступ и обход ---
		[[nodiscard]] size_t GetObjectCount() const noexcept;
		[[nodiscard]] std::span<GameObject* const> GetObjects() const noexcept;

		// --- Кадровый цикл логики ---
		void Update(float dt);

		// --- Связка с пространственным хранилищем слоя ---
		void SetStorage(ISpatialStorage* storage) noexcept { m_Storage = storage; }
		[[nodiscard]] ISpatialStorage* GetStorage() const noexcept { return m_Storage; }

	private:
		// Владение памятью: стабильные адреса объектов с гарантированным O(1) удалением
		std::unordered_map<GameObject*, std::unique_ptr<GameObject>> m_AllocatedObjects;

		// Плотный пул активных указателей для кэш-локального обхода Update()
		::zzz::core::SlotMap<GameObject*> m_ActiveObjects;

		// Указатель на пространственное хранилище слоя (для регистрации/дерегистрации)
		ISpatialStorage* m_Storage{ nullptr };
	};
}
