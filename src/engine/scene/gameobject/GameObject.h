#pragma once

#include <string>
#include <vector>
#include <memory>
#include "core/utils/Guid.h"
#include "core/templates/SlotMap.h"
#include "engine/scene/storage/ISceneTreeAccessor.h"

namespace zzz::core
{
	class Script;
}

namespace zzz
{
	using ::zzz::engine::NodeHandle;
	using ::zzz::engine::ISceneTreeAccessor;

	/**
	 * @class GameObject
	 * @brief Легковесный фасад сущности сцены, объединяющий ISceneTreeAccessor, ресурсы и скрипты.
	 */
	class GameObject final
	{
	public:
		explicit GameObject(std::string name = "GameObject");
		GameObject(::zzz::core::Guid guid, std::string name);
		~GameObject() = default;

		Z_NO_COPY_MOVE(GameObject);

		// --- Привязка к контейнеру сцены ---
		void BindSceneTree(ISceneTreeAccessor* tree, NodeHandle handle) noexcept
		{
			m_SceneTree = tree;
			m_NodeHandle = handle;
		}

		[[nodiscard]] NodeHandle GetNodeHandle() const noexcept { return m_NodeHandle; }
		[[nodiscard]] ISceneTreeAccessor* GetSceneTree() const noexcept { return m_SceneTree; }

		// --- Идентификация ---
		[[nodiscard]] const ::zzz::core::Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept;
		void SetName(std::string name);

		// --- Хэндл в ObjectWorld (O(1) удаление из SlotMap) ---
		[[nodiscard]] ::zzz::core::SlotHandle GetWorldHandle() const noexcept { return m_WorldHandle; }
		void SetWorldHandle(::zzz::core::SlotHandle handle) noexcept { m_WorldHandle = handle; }

		// --- Хэндл в пространственном хранилище ---
		[[nodiscard]] uint32_t GetSpatialHandle() const noexcept;
		void SetSpatialHandle(uint32_t handle) noexcept;

		// --- Активность и жизненный цикл в кадре ---
		[[nodiscard]] bool IsActive() const noexcept;
		void SetActive(bool active) noexcept;

		[[nodiscard]] uint8_t GetRenderFramesRemaining() const noexcept { return m_RenderFramesRemaining; }
		void DecrementRenderFrames() noexcept { if (m_RenderFramesRemaining > 0) --m_RenderFramesRemaining; }
		void ResetRenderFrames(uint8_t bufferCount = 2) noexcept { m_RenderFramesRemaining = bufferCount; }

		// --- Пространственные трансформации (делегирование в ISceneTreeAccessor) ---
		void SetLocalPosition(const ::zzz::math::Vec3<zF32>& pos);
		[[nodiscard]] const ::zzz::math::Vec3<zF32>& GetLocalPosition() const;

		void SetLocalRotation(const ::zzz::math::Quat<zF32>& rot);
		[[nodiscard]] const ::zzz::math::Quat<zF32>& GetLocalRotation() const;

		void SetLocalScale(const ::zzz::math::Vec3<zF32>& scale);
		[[nodiscard]] const ::zzz::math::Vec3<zF32>& GetLocalScale() const;

		[[nodiscard]] const ::zzz::math::Mat4<zF32>& GetWorldMatrix() const;

		// --- Иерархия сцены ---
		[[nodiscard]] GameObject* GetParent() const noexcept;
		void SetParent(GameObject* newParent, bool keepWorldTransform = true) noexcept;

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
		::zzz::core::SlotHandle m_WorldHandle{};
		uint8_t m_RenderFramesRemaining{ 2 };

		ISceneTreeAccessor* m_SceneTree{ nullptr };
		NodeHandle          m_NodeHandle{};

		std::string         m_FallbackName;

		::zzz::core::Guid m_MeshGuid;
		::zzz::core::Guid m_MaterialGuid;

		std::vector<std::shared_ptr<::zzz::core::Script>> m_Scripts;
	};
}
