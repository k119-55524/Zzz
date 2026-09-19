#pragma once

#include <span>
#include <string>
#include <vector>
#include <memory>
#include <expected>
#include <functional>

#include "core/utils/Guid.h"
#include "engine/scene/storage/NodeStorage.h"

namespace zzz::core
{
	class Script;
	class GameObjectData;
	class ScriptFactory;
}

#include "engine/resources/ResourceTypes.h"

namespace zzz::engine
{
	class GpuMesh;
	class GpuMaterial;

	/**
	 * @struct RenderPair
	 * @brief Пара привязки геометрии и материала для отрисовки.
	 */
	struct RenderPair
	{
		core::Guid meshGuid;
		core::Guid materialGuid;
		std::shared_ptr<GpuMesh> gpuMesh;
		std::shared_ptr<GpuMaterial> gpuMaterial;
	};
}

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::engine
{
	/**
	 * @class GameObject
	 * @brief Легковесный фасад сущности сцены, объединяющий идентификацию, NodeStorage и скрипты.
	 */
	class GameObject final : public std::enable_shared_from_this<GameObject>
	{
		Z_NO_COPY_MOVE(GameObject);

	public:
		explicit GameObject(const Guid& guid, std::string name, NodeStorage& storage, NodeHandle nodeHandle);
		~GameObject() = default;

		// Полная инициализация и наполнение объекта навешанными данными (скрипты, ресурсы)
		void Initialize(
			const GameObjectData& data,
			const ScriptFactory& scriptFactory,
			CoreGpuResourceManager& gpuResourceManager,
			std::function<void(std::expected<void, std::string>)> onReady);

#pragma region Getters and Setters
		[[nodiscard]] NodeHandle GetNodeHandle() const noexcept { return m_NodeHandle; }
		[[nodiscard]] NodeStorage* GetNodeStorage() const noexcept { return m_NodeStorage; }

		// --- Идентификация ---
		[[nodiscard]] const Guid& GetGuid() const noexcept { return m_Guid; }
		[[nodiscard]] const std::string& GetName() const noexcept { return m_Name; }

		// --- Хэндл в пространственном хранилище ---
		[[nodiscard]] SpatialHandle GetSpatialHandle() const
		{
			return m_NodeStorage->GetSpatialHandle(m_NodeHandle);
		}

		// --- Активность и жизненный цикл в кадре ---
		[[nodiscard]] bool IsActive() const
		{
			return m_NodeStorage->IsActive(m_NodeHandle);
		}
		void SetActive(bool active)
		{
			m_NodeStorage->SetActive(m_NodeHandle, active);
		}

		// --- Видимость для рендера ---
		[[nodiscard]] bool IsVisible() const
		{
			return m_NodeStorage->IsVisible(m_NodeHandle);
		}
		void SetVisible(bool visible)
		{
			m_NodeStorage->SetVisible(m_NodeHandle, visible);
		}

		// --- Пространственные трансформации (делегирование в NodeStorage) ---
		void SetLocalPosition(const Vec3<zF32>& pos)
		{
			m_NodeStorage->SetLocalPosition(m_NodeHandle, pos);
		}
		[[nodiscard]] Vec3<zF32> GetLocalPosition() const
		{
			return m_NodeStorage->GetLocalPosition(m_NodeHandle);
		}

		void SetLocalRotation(const Quat<zF32>& rot)
		{
			m_NodeStorage->SetLocalRotation(m_NodeHandle, rot);
		}
		[[nodiscard]] Quat<zF32> GetLocalRotation() const
		{
			return m_NodeStorage->GetLocalRotation(m_NodeHandle);
		}

		void SetLocalScale(const Vec3<zF32>& scale)
		{
			m_NodeStorage->SetLocalScale(m_NodeHandle, scale);
		}
		[[nodiscard]] Vec3<zF32> GetLocalScale() const
		{
			return m_NodeStorage->GetLocalScale(m_NodeHandle);
		}

		[[nodiscard]] Mat4<zF32> GetLocalMatrix() const
		{
			return m_NodeStorage->GetLocalMatrix(m_NodeHandle);
		}
		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix() const
		{
			return m_NodeStorage->GetWorldMatrix(m_NodeHandle);
		}

		// --- Иерархия сцены ---
		[[nodiscard]] NodeHandle GetParent() const
		{
			return m_NodeStorage->GetParent(m_NodeHandle);
		}

		// --- Меши и материалы (пары рендера) ---
		[[nodiscard]] std::span<const RenderPair> GetRenderPairs() const noexcept { return m_RenderPairs; }
		[[nodiscard]] bool HasRenderPairs() const noexcept { return !m_RenderPairs.empty(); }
		[[nodiscard]] size_t GetRenderPairCount() const noexcept { return m_RenderPairs.size(); }

		[[nodiscard]] const Guid& GetMeshGuid(size_t index = 0) const noexcept
		{
			return index < m_RenderPairs.size() ? m_RenderPairs[index].meshGuid : s_EmptyGuid;
		}
		[[nodiscard]] const Guid& GetMaterialGuid(size_t index = 0) const noexcept
		{
			return index < m_RenderPairs.size() ? m_RenderPairs[index].materialGuid : s_EmptyGuid;
		}
		[[nodiscard]] std::shared_ptr<GpuMesh> GetGpuMesh(size_t index = 0) const noexcept
		{
			return index < m_RenderPairs.size() ? m_RenderPairs[index].gpuMesh : nullptr;
		}
		[[nodiscard]] std::shared_ptr<GpuMaterial> GetGpuMaterial(size_t index = 0) const noexcept
		{
			return index < m_RenderPairs.size() ? m_RenderPairs[index].gpuMaterial : nullptr;
		}
		[[nodiscard]] bool HasMesh() const noexcept
		{
			return !m_RenderPairs.empty() && m_RenderPairs[0].meshGuid.IsValid();
		}
		[[nodiscard]] bool HasMaterial() const noexcept
		{
			return !m_RenderPairs.empty() && m_RenderPairs[0].materialGuid.IsValid();
		}

		[[nodiscard]] const std::vector<std::shared_ptr<Script>>& GetScripts() const noexcept { return m_Scripts; }
#pragma endregion

		void AddScript(std::shared_ptr<Script> script);

	private:
		Guid m_Guid;
		std::string m_Name;

		NodeStorage* m_NodeStorage;
		NodeHandle m_NodeHandle;

		std::vector<RenderPair> m_RenderPairs;
		std::vector<std::shared_ptr<Script>> m_Scripts;
		static inline const Guid s_EmptyGuid{};
	};
}
