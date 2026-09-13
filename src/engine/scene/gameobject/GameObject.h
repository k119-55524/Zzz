#pragma once

#include <string>
#include <vector>
#include <memory>

#include "core/utils/Guid.h"
#include "engine/scene/storage/NodeStorage.h"
#include "engine/scene/visual/VisualTypes.h"

namespace zzz::core
{
	class Script;
	class GameObjectData;
	class ScriptFactory;
}

namespace zzz::engine
{
	class ResourceManager;
}

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::engine
{
	/**
	 * @class GameObject
	 * @brief Легковесный фасад сущности сцены, объединяющий NodeStorage, ресурсы и скрипты.
	 */
	class GameObject final
	{
		Z_NO_COPY_MOVE(GameObject);

	public:
		explicit GameObject(const Guid& guid, std::string name, VisualPayload visual = {});
		~GameObject() = default;

		// Полная инициализация и наполнение объекта навешанными данными (трансформ, скрипты, ресурсы)
		void Initialize(
			const GameObjectData& data,
			const ScriptFactory& scriptFactory,
			ResourceManager& resourceManager,
			NodeStorage* storage,
			NodeHandle nodeHandle);

		// Привязка к контейнеру сцены
		void BindNodeStorage(NodeStorage* storage, NodeHandle nodeHandle);

#pragma region Getters and Setters
		[[nodiscard]] NodeHandle GetNodeHandle() const noexcept { return m_NodeHandle; }
		[[nodiscard]] NodeStorage* GetNodeStorage() const noexcept { return m_NodeStorage; }

		// --- Идентификация ---
		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetName(std::string name) { m_Name = std::move(name); }

		// --- Хэндл в пространственном хранилище ---
		[[nodiscard]] SpatialHandle GetSpatialHandle() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetSpatialHandle(m_NodeHandle);
		}

		// --- Активность и жизненный цикл в кадре ---
		[[nodiscard]] bool IsActive() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->IsActive(m_NodeHandle);
		}
		void SetActive(bool active)
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			m_NodeStorage->SetActive(m_NodeHandle, active);
		}

		// --- Пространственные трансформации (делегирование в NodeStorage) ---
		void SetLocalPosition(const Vec3<zF32>& pos)
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			m_NodeStorage->SetLocalPosition(m_NodeHandle, pos);
		}
		[[nodiscard]] Vec3<zF32> GetLocalPosition() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetLocalPosition(m_NodeHandle);
		}

		void SetLocalRotation(const Quat<zF32>& rot)
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			m_NodeStorage->SetLocalRotation(m_NodeHandle, rot);
		}
		[[nodiscard]] Quat<zF32> GetLocalRotation() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetLocalRotation(m_NodeHandle);
		}

		void SetLocalScale(const Vec3<zF32>& scale)
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			m_NodeStorage->SetLocalScale(m_NodeHandle, scale);
		}
		[[nodiscard]] Vec3<zF32> GetLocalScale() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetLocalScale(m_NodeHandle);
		}

		[[nodiscard]] Mat4<zF32> GetLocalMatrix() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetLocalMatrix(m_NodeHandle);
		}
		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetWorldMatrix(m_NodeHandle);
		}

		// --- Иерархия сцены ---
		[[nodiscard]] NodeHandle GetParent() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetParent(m_NodeHandle);
		}

		// --- Визуальная нагрузка ---
		[[nodiscard]] const VisualPayload& GetVisual() const noexcept { return m_Visual; }
		void SetVisual(VisualPayload visual) noexcept { m_Visual = std::move(visual); }

		[[nodiscard]] const std::vector<std::shared_ptr<Script>>& GetScripts() const noexcept { return m_Scripts; }
#pragma endregion

		void AddScript(std::shared_ptr<Script> script);

	private:
		Guid m_Guid;
		std::string  m_Name;

		NodeStorage* m_NodeStorage;
		NodeHandle m_NodeHandle;

		VisualPayload m_Visual;
		std::vector<std::shared_ptr<Script>> m_Scripts;
	};
}
