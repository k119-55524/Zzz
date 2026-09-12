#pragma once

#include <span>
#include <vector>
#include <cstdint>

#include "core/utils/Ensure.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/containers/BitTreeTracker.h"
#include "engine/scene/storage/NodeTypes.h"

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
		void SetLocalPosition(zU32 nodeIndex, const Vec3<zF32>& pos)
		{
			ensure(IsValid(nodeIndex), "NodeStorage::SetLocalPosition: невалидный nodeIndex");
			m_LocalTransforms[nodeIndex].position = pos;
			MarkDirty(nodeIndex);
		}

		[[nodiscard]] Vec3<zF32> GetLocalPosition(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::GetLocalPosition: невалидный nodeIndex");
			return m_LocalTransforms[nodeIndex].position;
		}

		void SetLocalRotation(zU32 nodeIndex, const Quat<zF32>& rot)
		{
			ensure(IsValid(nodeIndex), "NodeStorage::SetLocalRotation: невалидный nodeIndex");
			m_LocalTransforms[nodeIndex].rotation = rot;
			MarkDirty(nodeIndex);
		}

		[[nodiscard]] Quat<zF32> GetLocalRotation(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::GetLocalRotation: невалидный nodeIndex");
			return m_LocalTransforms[nodeIndex].rotation;
		}

		void SetLocalScale(zU32 nodeIndex, const Vec3<zF32>& scale)
		{
			ensure(IsValid(nodeIndex), "NodeStorage::SetLocalScale: невалидный nodeIndex");
			m_LocalTransforms[nodeIndex].scale = scale;
			MarkDirty(nodeIndex);
		}

		[[nodiscard]] Vec3<zF32> GetLocalScale(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::GetLocalScale: невалидный nodeIndex");
			return m_LocalTransforms[nodeIndex].scale;
		}

		[[nodiscard]] const Mat4<zF32>& GetLocalMatrix(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::GetLocalMatrix: невалидный nodeIndex");
			return m_LocalMatrices[nodeIndex];
		}

		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::GetWorldMatrix: невалидный nodeIndex");
			return m_WorldMatrices[nodeIndex];
		}

		// --- Топология и иерархия ---
		[[nodiscard]] zU32 GetParent(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::GetParent: невалидный nodeIndex");
			return m_Topology[nodeIndex].parentIndex;
		}

		void MarkDirty(zU32 nodeIndex)
		{
			ensure(IsValid(nodeIndex), "NodeStorage::MarkDirty: индекс узла выходит за пределы хранилища");
			m_DirtyTracker.Set(nodeIndex);
		}

		// --- Свойства узла ---
		void SetActive(zU32 nodeIndex, bool active)
		{
			ensure(IsValid(nodeIndex), "NodeStorage::SetActive: невалидный nodeIndex");
			m_States[nodeIndex].isActive = active;
		}

		[[nodiscard]] bool IsActive(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::IsActive: невалидный nodeIndex");
			return m_States[nodeIndex].isActive;
		}

		void SetSpatialHandle(zU32 nodeIndex, zU32 spHandle)
		{
			ensure(IsValid(nodeIndex), "NodeStorage::SetSpatialHandle: невалидный nodeIndex");
			m_Bindings[nodeIndex].spatialHandle = spHandle;
		}

		[[nodiscard]] zU32 GetSpatialHandle(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::GetSpatialHandle: невалидный nodeIndex");
			return m_Bindings[nodeIndex].spatialHandle;
		}

		// --- Валидация ---
		[[nodiscard]] bool IsValid(zU32 nodeIndex) const noexcept
		{
			return nodeIndex < m_States.size();
		}

		[[nodiscard]] zU32 GetLayerObjectIndex(zU32 nodeIndex) const
		{
			ensure(IsValid(nodeIndex), "NodeStorage::GetLayerObjectIndex: невалидный nodeIndex");
			return m_Bindings[nodeIndex].layerObjectIndex;
		}

		// --- Доступ к размерам и плоским SoA данным для подсистем (SpatialStorage, Renderer) ---
		[[nodiscard]] size_t GetNodeCount() const noexcept { return m_States.size(); }
		[[nodiscard]] std::span<const Mat4<zF32>> GetWorldMatrices() const noexcept { return m_WorldMatrices; }
		[[nodiscard]] std::span<const NodeState> GetStates() const noexcept { return m_States; }
		[[nodiscard]] std::span<const NodeBindings> GetBindings() const noexcept { return m_Bindings; }
		[[nodiscard]] std::span<const Transform> GetLocalTransforms() const noexcept { return m_LocalTransforms; }
#pragma endregion

	private:
		void ResolveSubtree(uint32_t nodeIndex, const Mat4<zF32>& parentWorld);

		std::vector<NodeTopology>	m_Topology;
		std::vector<Transform>		m_LocalTransforms;
		std::vector<Mat4<zF32>>		m_LocalMatrices;
		std::vector<Mat4<zF32>>		m_WorldMatrices;
		std::vector<NodeState>		m_States;
		std::vector<NodeBindings>	m_Bindings;

		BitTreeTracker m_DirtyTracker;
	};
}
