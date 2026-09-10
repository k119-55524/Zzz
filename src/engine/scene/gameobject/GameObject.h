#pragma once

#include <string>
#include <vector>
#include <memory>

#include "core/utils/Guid.h"
#include "engine/scene/storage/ISceneTreeAccessor.h"

namespace zzz::core
{
	class Script;
}

using namespace zzz::math;

namespace zzz::engine
{
	/**
	 * @class GameObject
	 * @brief Легковесный фасад сущности сцены, объединяющий ISceneTreeAccessor, ресурсы и скрипты.
	 */
	class GameObject final
	{
		Z_NO_COPY_MOVE(GameObject);

	public:
		explicit GameObject(const Guid& guid, std::string name);
		~GameObject() = default;

		// --- Привязка к контейнеру сцены ---
		void BindSceneTree(ISceneTreeAccessor* tree, NodeHandle handle) noexcept
		{
			m_SceneTree = tree;
			m_NodeHandle = handle;
		}

		[[nodiscard]] NodeHandle GetNodeHandle() const noexcept { return m_NodeHandle; }
		[[nodiscard]] ISceneTreeAccessor* GetSceneTree() const noexcept { return m_SceneTree; }

		// --- Идентификация ---
		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept;
		void SetName(std::string name);

		// --- Хэндл в пространственном хранилище ---
		[[nodiscard]] uint32_t GetSpatialHandle() const noexcept;
		void SetSpatialHandle(uint32_t handle) noexcept;

		// --- Активность и жизненный цикл в кадре ---
		[[nodiscard]] bool IsActive() const noexcept;
		void SetActive(bool active) noexcept;

		// --- Пространственные трансформации (делегирование в ISceneTreeAccessor) ---
		void SetLocalPosition(const Vec3<zF32>& pos);
		[[nodiscard]] const Vec3<zF32>& GetLocalPosition() const;

		void SetLocalRotation(const Quat<zF32>& rot);
		[[nodiscard]] const Quat<zF32>& GetLocalRotation() const;

		void SetLocalScale(const Vec3<zF32>& scale);
		[[nodiscard]] const Vec3<zF32>& GetLocalScale() const;

		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix() const;

		// --- Иерархия сцены ---
		[[nodiscard]] GameObject* GetParent() const noexcept;
		void SetParent(GameObject* newParent, bool keepWorldTransform = true) noexcept;

		// --- Слоты графических ресурсов (для отрисовки меша и материала) ---
		[[nodiscard]] const Guid& GetMeshGuid() const noexcept { return m_MeshGuid; }
		void SetMeshGuid(const Guid& guid) noexcept { m_MeshGuid = guid; }
		[[nodiscard]] bool HasMesh() const noexcept { return !m_MeshGuid.IsEmpty(); }

		[[nodiscard]] const Guid& GetMaterialGuid() const noexcept { return m_MaterialGuid; }
		void SetMaterialGuid(const Guid& guid) noexcept { m_MaterialGuid = guid; }
		[[nodiscard]] bool HasMaterial() const noexcept { return !m_MaterialGuid.IsEmpty(); }

		// --- Скрипты поведения ---
		void AddScript(std::shared_ptr<Script> script);
		void RemoveScript(const std::shared_ptr<Script>& script);
		void RemoveAllScripts();
		[[nodiscard]] const std::vector<std::shared_ptr<Script>>& GetScripts() const noexcept { return m_Scripts; }

	private:
		Guid m_Guid;

		ISceneTreeAccessor* m_SceneTree{ nullptr };
		NodeHandle          m_NodeHandle{};

		std::string         m_FallbackName;

		Guid m_MeshGuid;
		Guid m_MaterialGuid;

		std::vector<std::shared_ptr<Script>> m_Scripts;
	};
}
