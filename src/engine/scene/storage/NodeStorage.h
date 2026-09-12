#pragma once

#include <span>
#include <vector>
#include <cstdint>

#include "core/utils/Ensure.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/containers/BitTreeTracker.h"
#include "engine/scene/storage/ISceneTreeAccessor.h"

using namespace zzz::math;
using namespace zzz::core;

namespace zzz::core
{
	class GameObjectData;
}

namespace zzz::engine
{
	/**
	 * @class NodeStorage
	 * @brief Плоское SoA-хранилище (Structure of Arrays) данных узлов слоя сцены.
	 *
	 * @details Инкапсулирует параллельные непрерывные векторы топологии, локальных TRS,
	 * локальных и мировых матриц, метаданных узлов и битовое дерево изменений (BitTreeTracker).
	 */
	class NodeStorage final
	{
		Z_NO_COPY(NodeStorage);

	public:
		NodeStorage(NodeStorage&&) noexcept = default;
		NodeStorage& operator=(NodeStorage&&) noexcept = default;

		NodeStorage();
		explicit NodeStorage(std::span<const zzz::core::GameObjectData> objects);
		~NodeStorage() = default;

		void BeginFrame();
		void ResolveTransforms();

#pragma region Getters and Setters
		// --- Пространственные координаты ---
		void SetLocalPosition(NodeHandle handle, const Vec3<zF32>& pos)
		{
			ensure(IsValid(handle), "NodeStorage::SetLocalPosition: невалидный NodeHandle");
			m_LocalTransforms[handle.index].position = pos;
			MarkDirty(handle);
		}

		[[nodiscard]] Vec3<zF32> GetLocalPosition(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLocalPosition: невалидный NodeHandle");
			return m_LocalTransforms[handle.index].position;
		}

		void SetLocalRotation(NodeHandle handle, const Quat<zF32>& rot)
		{
			ensure(IsValid(handle), "NodeStorage::SetLocalRotation: невалидный NodeHandle");
			m_LocalTransforms[handle.index].rotation = rot;
			MarkDirty(handle);
		}

		[[nodiscard]] Quat<zF32> GetLocalRotation(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLocalRotation: невалидный NodeHandle");
			return m_LocalTransforms[handle.index].rotation;
		}

		void SetLocalScale(NodeHandle handle, const Vec3<zF32>& scale)
		{
			ensure(IsValid(handle), "NodeStorage::SetLocalScale: невалидный NodeHandle");
			m_LocalTransforms[handle.index].scale = scale;
			MarkDirty(handle);
		}

		[[nodiscard]] Vec3<zF32> GetLocalScale(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLocalScale: невалидный NodeHandle");
			return m_LocalTransforms[handle.index].scale;
		}

		[[nodiscard]] const Mat4<zF32>& GetLocalMatrix(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLocalMatrix: невалидный NodeHandle");
			return m_LocalMatrices[handle.index];
		}

		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetWorldMatrix: невалидный NodeHandle");
			return m_WorldMatrices[handle.index];
		}

		// --- Топология и иерархия ---
		void SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform = true);
		[[nodiscard]] NodeHandle GetParent(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetParent: невалидный NodeHandle");
			const uint32_t pIdx = m_Topology[handle.index].parentIndex;
			if (pIdx != 0xFFFFFFFF && pIdx < m_Metadata.size())
			{
				return NodeHandle{ pIdx, m_Metadata[pIdx].generation };
			}
			return NodeHandle{};
		}

		[[nodiscard]] GameObject* GetNodeOwner(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetNodeOwner: невалидный NodeHandle");
			return m_Metadata[handle.index].gameObject;
		}

		void SetNodeOwner(NodeHandle handle, GameObject* owner)
		{
			ensure(IsValid(handle), "NodeStorage::SetNodeOwner: невалидный NodeHandle");
			m_Metadata[handle.index].gameObject = owner;
			m_Metadata[handle.index].payloadType = owner ? SceneNodeType::GameObject : SceneNodeType::Entity;
		}

		void MarkDirty(NodeHandle handle)
		{
			ensure(IsValid(handle), "NodeStorage::MarkDirty: невалидный NodeHandle");
			MarkDirty(handle.index);
		}

		// --- Свойства узла ---
		void SetActive(NodeHandle handle, bool active)
		{
			ensure(IsValid(handle), "NodeStorage::SetActive: невалидный NodeHandle");
			m_Metadata[handle.index].isActive = active;
		}

		[[nodiscard]] bool IsActive(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::IsActive: невалидный NodeHandle");
			return m_Metadata[handle.index].isActive;
		}

		void SetSpatialHandle(NodeHandle handle, SpatialHandle spHandle)
		{
			ensure(IsValid(handle), "NodeStorage::SetSpatialHandle: невалидный NodeHandle");
			m_Metadata[handle.index].spatialHandle = spHandle;
		}

		[[nodiscard]] SpatialHandle GetSpatialHandle(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetSpatialHandle: невалидный NodeHandle");
			return m_Metadata[handle.index].spatialHandle;
		}

		// --- Валидация ---
		[[nodiscard]] bool IsValid(NodeHandle handle) const noexcept
		{
			return handle.index < m_Metadata.size()
				&& m_Metadata[handle.index].isAlive
				&& m_Metadata[handle.index].generation == handle.generation;
		}

		[[nodiscard]] NodeHandle GetHandle(uint32_t storageIndex) const noexcept
		{
			if (storageIndex < m_Metadata.size() && m_Metadata[storageIndex].isAlive)
			{
				return NodeHandle{ storageIndex, m_Metadata[storageIndex].generation };
			}
			return NodeHandle{};
		}

		[[nodiscard]] uint32_t GetLayerObjectIndex(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetLayerObjectIndex: невалидный NodeHandle");
			return m_Metadata[handle.index].layerObjectIndex;
		}

		[[nodiscard]] SceneNodeType GetNodeType(NodeHandle handle) const
		{
			ensure(IsValid(handle), "NodeStorage::GetNodeType: невалидный NodeHandle");
			return m_Metadata[handle.index].payloadType;
		}

		// --- Доступ к размерам и плоским SoA данным для подсистем (SpatialStorage, Renderer) ---
		[[nodiscard]] size_t GetNodeCount() const noexcept { return m_Metadata.size(); }
		[[nodiscard]] std::span<const Mat4<zF32>> GetWorldMatrices() const noexcept { return m_WorldMatrices; }
		[[nodiscard]] std::span<const NodeMetadata> GetMetadata() const noexcept { return m_Metadata; }
		[[nodiscard]] std::span<const LocalTransform> GetLocalTransforms() const noexcept { return m_LocalTransforms; }
#pragma endregion

	private:
		inline void MarkDirty(uint32_t nodeIndex)
		{
			ensure(nodeIndex < m_Metadata.size(), "NodeStorage::MarkDirty: индекс узла выходит за пределы metadata");
			m_DirtyTracker.Set(nodeIndex);
		}

		void ResolveSubtree(uint32_t nodeIndex, const Mat4<zF32>& parentWorld);

	private:
		std::vector<NodeTopology>   m_Topology;
		std::vector<LocalTransform> m_LocalTransforms;
		std::vector<Mat4<zF32>>     m_LocalMatrices;
		std::vector<Mat4<zF32>>     m_WorldMatrices;
		std::vector<NodeMetadata>   m_Metadata;
		BitTreeTracker              m_DirtyTracker;
	};
}
