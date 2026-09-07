#pragma once

#include <string>
#include <vector>
#include <memory>
#include "core/utils/Guid.h"
#include "core/templates/SlotMap.h"
#include "engine/scene/gameobject/Transform.h"

namespace zzz::core
{
	class Script;
}

namespace zzz
{
	/**
	 * @class GameObject
	 * @brief Сущность игрового мира, объединяющая иерархию сцены, Transform, ресурсы и скрипты.
	 */
	class GameObject final
	{
	public:
		explicit GameObject(std::string name = "GameObject");
		GameObject(::zzz::core::Guid guid, std::string name);
		~GameObject() = default;

		Z_NO_COPY_MOVE(GameObject);

		// --- Идентификация ---
		[[nodiscard]] const ::zzz::core::Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetName(std::string name) { m_Name = std::move(name); }

		// --- Хэндл в ObjectWorld (O(1) удаление из SlotMap) ---
		[[nodiscard]] ::zzz::core::SlotHandle GetWorldHandle() const noexcept { return m_WorldHandle; }
		void SetWorldHandle(::zzz::core::SlotHandle handle) noexcept { m_WorldHandle = handle; }

		// --- Хэндл в пространственном хранилище ---
		[[nodiscard]] uint32_t GetSpatialHandle() const noexcept { return m_SpatialHandle; }
		void SetSpatialHandle(uint32_t handle) noexcept { m_SpatialHandle = handle; }

		// --- Активность и жизненный цикл в кадре ---
		[[nodiscard]] bool IsActive() const noexcept { return m_IsActive; }
		void SetActive(bool active) noexcept { m_IsActive = active; }

		[[nodiscard]] uint8_t GetRenderFramesRemaining() const noexcept { return m_RenderFramesRemaining; }
		void DecrementRenderFrames() noexcept { if (m_RenderFramesRemaining > 0) --m_RenderFramesRemaining; }
		void ResetRenderFrames(uint8_t bufferCount = 2) noexcept { m_RenderFramesRemaining = bufferCount; }

		// --- Пространственная трансформация ---
		[[nodiscard]] Transform& GetTransform() noexcept { return m_Transform; }
		[[nodiscard]] const Transform& GetTransform() const noexcept { return m_Transform; }

		// --- Иерархия сцены (Parent / Children) ---
		[[nodiscard]] GameObject* GetParent() const noexcept { return m_Parent; }
		void SetParent(GameObject* newParent, bool keepWorldTransform = true) noexcept;

		[[nodiscard]] const std::vector<GameObject*>& GetChildren() const noexcept { return m_Children; }
		[[nodiscard]] size_t GetChildCount() const noexcept { return m_Children.size(); }
		[[nodiscard]] GameObject* GetChild(size_t index) const noexcept;

		// --- Слоты графических ресурсов (для отрисовки меша и материала) ---
		[[nodiscard]] const ::zzz::core::Guid& GetMeshGuid() const noexcept { return m_MeshGuid; }
		void SetMeshGuid(const ::zzz::core::Guid& guid) noexcept { m_MeshGuid = guid; }
		[[nodiscard]] bool HasMesh() const noexcept { return !m_MeshGuid.IsEmpty(); }

		[[nodiscard]] const ::zzz::core::Guid& GetMaterialGuid() const noexcept { return m_MaterialGuid; }
		void SetMaterialGuid(const ::zzz::core::Guid& guid) noexcept { m_MaterialGuid = guid; }
		[[nodiscard]] bool HasMaterial() const noexcept { return !m_MaterialGuid.IsEmpty(); }

		// --- Скрипты поведения ---
		void AddScript(std::shared_ptr<::zzz::core::Script> script);
		void RemoveScript(const std::shared_ptr<::zzz::core::Script>& script);
		void RemoveAllScripts();
		[[nodiscard]] const std::vector<std::shared_ptr<::zzz::core::Script>>& GetScripts() const noexcept { return m_Scripts; }

	private:
		::zzz::core::Guid m_Guid;
		std::string m_Name;
		::zzz::core::SlotHandle m_WorldHandle{};
		uint32_t m_SpatialHandle{ 0xFFFFFFFF };
		bool m_IsActive{ true };
		uint8_t m_RenderFramesRemaining{ 2 }; // Frames in Flight safety

		Transform m_Transform;

		GameObject* m_Parent{ nullptr };
		std::vector<GameObject*> m_Children;

		::zzz::core::Guid m_MeshGuid;
		::zzz::core::Guid m_MaterialGuid;

		std::vector<std::shared_ptr<::zzz::core::Script>> m_Scripts;
	};
}
