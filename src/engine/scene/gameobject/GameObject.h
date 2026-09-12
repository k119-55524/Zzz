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

		// Привязка к контейнеру сцены
		void BindNodeStorage(NodeStorage* storage, zU32 nodeIndex);

#pragma region Getters and Setters
		[[nodiscard]] zU32 GetNodeIndex() const noexcept { return m_NodeIndex; }
		[[nodiscard]] NodeStorage* GetNodeStorage() const noexcept { return m_NodeStorage; }

		// --- Идентификация ---
		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetName(std::string name) { m_Name = std::move(name); }

		// --- Хэндл в пространственном хранилище ---
		[[nodiscard]] zU32 GetSpatialHandle() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetSpatialHandle(m_NodeIndex);
		}

		// --- Активность и жизненный цикл в кадре ---
		[[nodiscard]] bool IsActive() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->IsActive(m_NodeIndex);
		}
		void SetActive(bool active)
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			m_NodeStorage->SetActive(m_NodeIndex, active);
		}

		// --- Пространственные трансформации (делегирование в NodeStorage) ---
		void SetLocalPosition(const Vec3<zF32>& pos)
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			m_NodeStorage->SetLocalPosition(m_NodeIndex, pos);
		}
		[[nodiscard]] Vec3<zF32> GetLocalPosition() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetLocalPosition(m_NodeIndex);
		}

		void SetLocalRotation(const Quat<zF32>& rot)
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			m_NodeStorage->SetLocalRotation(m_NodeIndex, rot);
		}
		[[nodiscard]] Quat<zF32> GetLocalRotation() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetLocalRotation(m_NodeIndex);
		}

		void SetLocalScale(const Vec3<zF32>& scale)
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			m_NodeStorage->SetLocalScale(m_NodeIndex, scale);
		}
		[[nodiscard]] Vec3<zF32> GetLocalScale() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetLocalScale(m_NodeIndex);
		}

		[[nodiscard]] Mat4<zF32> GetLocalMatrix() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetLocalMatrix(m_NodeIndex);
		}
		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetWorldMatrix(m_NodeIndex);
		}

		// --- Иерархия сцены ---
		[[nodiscard]] zU32 GetParentIndex() const
		{
			ensure(m_NodeStorage != nullptr, "GameObject '{}': обращение к NodeStorage до BindNodeStorage", m_Name);
			return m_NodeStorage->GetParent(m_NodeIndex);
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
		zU32 m_NodeIndex;

		VisualPayload m_Visual;
		std::vector<std::shared_ptr<Script>> m_Scripts;
	};
}
