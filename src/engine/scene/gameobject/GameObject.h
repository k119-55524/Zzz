#pragma once

#include <string>
#include <vector>
#include <memory>

#include "core/utils/Guid.h"
#include "engine/scene/storage/NodeTypes.h"
#include "engine/scene/visual/VisualTypes.h"

namespace zzz::core
{
	class Script;
}

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::engine
{
	class NodeStorage;

	/**
	 * @class GameObject
	 * @brief Легковесный фасад сущности сцены, объединяющий NodeStorage, ресурсы и скрипты.
	 */
	class GameObject final
	{
		Z_NO_COPY_MOVE(GameObject);

	public:
		explicit GameObject(const Guid& guid, std::string name);
		~GameObject() = default;

		// --- Привязка к контейнеру сцены ---
		void BindNodeStorage(NodeStorage* storage, zU32 nodeIndex) noexcept
		{
			m_NodeStorage = storage;
			m_NodeIndex = nodeIndex;
		}

		[[nodiscard]] zU32 GetNodeIndex() const noexcept { return m_NodeIndex; }
		[[nodiscard]] NodeStorage* GetNodeStorage() const noexcept { return m_NodeStorage; }

		// --- Идентификация ---
		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }
		void SetName(std::string name) { m_Name = std::move(name); }

		// --- Хэндл в пространственном хранилище ---
		[[nodiscard]] zU32 GetSpatialHandle() const;

		// --- Активность и жизненный цикл в кадре ---
		[[nodiscard]] bool IsActive() const;
		void SetActive(bool active);

		// --- Пространственные трансформации (делегирование в NodeStorage) ---
		void SetLocalPosition(const Vec3<zF32>& pos);
		[[nodiscard]] Vec3<zF32> GetLocalPosition() const;

		void SetLocalRotation(const Quat<zF32>& rot);
		[[nodiscard]] Quat<zF32> GetLocalRotation() const;

		void SetLocalScale(const Vec3<zF32>& scale);
		[[nodiscard]] Vec3<zF32> GetLocalScale() const;

		[[nodiscard]] Mat4<zF32> GetLocalMatrix() const;
		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix() const;

		// --- Иерархия сцены ---
		[[nodiscard]] zU32 GetParentIndex() const;

		// --- Визуальная нагрузка ---
		[[nodiscard]] eVisualType GetVisualType() const noexcept { return m_Visual.type; }
		[[nodiscard]] bool HasVisual() const noexcept { return m_Visual.type != eVisualType::None; }
		[[nodiscard]] const VisualPayload& GetVisual() const noexcept { return m_Visual; }
		void SetVisual(VisualPayload visual) noexcept { m_Visual = std::move(visual); }

		// --- Совместимые хелперы ---
		[[nodiscard]] const Guid& GetMeshGuid() const noexcept { return m_Visual.resourceGuid; }
		void SetMeshGuid(const Guid& guid) noexcept
		{
			m_Visual.resourceGuid = guid;
			if (m_Visual.type == eVisualType::None)
			{
				m_Visual.type = eVisualType::SimpleMesh3D;
			}
		}
		[[nodiscard]] bool HasMesh() const noexcept
		{
			return (m_Visual.type == eVisualType::SimpleMesh3D || m_Visual.type == eVisualType::Mesh2D) && !m_Visual.resourceGuid.IsEmpty();
		}

		[[nodiscard]] const Guid& GetMaterialGuid() const noexcept { return m_Visual.materialGuid; }
		void SetMaterialGuid(const Guid& guid) noexcept { m_Visual.materialGuid = guid; }
		[[nodiscard]] bool HasMaterial() const noexcept { return !m_Visual.materialGuid.IsEmpty(); }

		// --- Скрипты поведения ---
		void AddScript(std::shared_ptr<Script> script);
		void RemoveScript(const std::shared_ptr<Script>& script);
		void RemoveAllScripts();
		[[nodiscard]] const std::vector<std::shared_ptr<Script>>& GetScripts() const noexcept { return m_Scripts; }

	private:
		Guid m_Guid;

		NodeStorage* m_NodeStorage{ nullptr };
		zU32         m_NodeIndex{ kInvalidNodeIndex };

		std::string  m_Name;

		VisualPayload m_Visual;

		std::vector<std::shared_ptr<Script>> m_Scripts;
	};
}
