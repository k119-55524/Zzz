#pragma once

#include <vector>

#include "core/utils/Ensure.h"
#include "core/utils/macros/MiscMacros.h"
#include "engine/scene/storage/NodeStorageBlock.h"

using namespace zzz::math;
using namespace zzz::core;

namespace zzz::engine
{
	class SceneTreeContainer final : public ISceneTreeAccessor
	{
		Z_NO_COPY_MOVE(SceneTreeContainer);

	public:
		SceneTreeContainer();
		~SceneTreeContainer() override = default;

		NodeHandle CreateNode(GameObject* owner = nullptr);

		void BeginFrame();
		void ResolveTransforms();

#pragma region Getters and Setters
		// --- ISceneTreeAccessor: Пространственные координаты ---
		void SetLocalPosition(NodeHandle handle, const Vec3<zF32>& pos) override
		{
			ensure(IsValid(handle), "SceneTreeContainer::SetLocalPosition: невалидный NodeHandle");
			m_Nodes.localTransforms[handle.index].position = pos;
			m_Nodes.MarkDirty(handle.index);
		}

		[[nodiscard]] Vec3<zF32> GetLocalPosition(NodeHandle handle) const override
		{
			ensure(IsValid(handle), "SceneTreeContainer::GetLocalPosition: невалидный NodeHandle");
			return m_Nodes.localTransforms[handle.index].position;
		}

		void SetLocalRotation(NodeHandle handle, const Quat<zF32>& rot) override
		{
			ensure(IsValid(handle), "SceneTreeContainer::SetLocalRotation: невалидный NodeHandle");
			m_Nodes.localTransforms[handle.index].rotation = rot;
			m_Nodes.MarkDirty(handle.index);
		}

		[[nodiscard]] Quat<zF32> GetLocalRotation(NodeHandle handle) const override
		{
			ensure(IsValid(handle), "SceneTreeContainer::GetLocalRotation: невалидный NodeHandle");
			return m_Nodes.localTransforms[handle.index].rotation;
		}

		void SetLocalScale(NodeHandle handle, const Vec3<zF32>& scale) override
		{
			ensure(IsValid(handle), "SceneTreeContainer::SetLocalScale: невалидный NodeHandle");
			m_Nodes.localTransforms[handle.index].scale = scale;
			m_Nodes.MarkDirty(handle.index);
		}

		[[nodiscard]] Vec3<zF32> GetLocalScale(NodeHandle handle) const override
		{
			ensure(IsValid(handle), "SceneTreeContainer::GetLocalScale: невалидный NodeHandle");
			return m_Nodes.localTransforms[handle.index].scale;
		}

		[[nodiscard]] const Mat4<zF32>& GetLocalMatrix(NodeHandle handle) const override
		{
			ensure(IsValid(handle), "SceneTreeContainer::GetLocalMatrix: невалидный NodeHandle");
			return m_Nodes.localMatrices[handle.index];
		}

		[[nodiscard]] const Mat4<zF32>& GetWorldMatrix(NodeHandle handle) const override
		{
			ensure(IsValid(handle), "SceneTreeContainer::GetWorldMatrix: невалидный NodeHandle");
			return m_Nodes.worldMatrices[handle.index];
		}

		// --- ISceneTreeAccessor: Топология и иерархия ---
		void SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform = true) override;
		[[nodiscard]] NodeHandle GetParent(NodeHandle handle) const override
		{
			ensure(IsValid(handle), "SceneTreeContainer::GetParent: невалидный NodeHandle");
			const uint32_t pIdx = m_Nodes.topology[handle.index].parentIndex;
			if (pIdx != 0xFFFFFFFF && pIdx < m_Nodes.metadata.size())
			{
				return NodeHandle{ pIdx, m_Nodes.metadata[pIdx].generation };
			}
			return NodeHandle{};
		}

		[[nodiscard]] GameObject* GetNodeOwner(NodeHandle handle) const override
		{
			ensure(IsValid(handle), "SceneTreeContainer::GetNodeOwner: невалидный NodeHandle");
			return m_Nodes.metadata[handle.index].gameObject;
		}

		void MarkDirty(NodeHandle handle) override
		{
			ensure(IsValid(handle), "SceneTreeContainer::MarkDirty: невалидный NodeHandle");
			m_Nodes.MarkDirty(handle.index);
		}

		// --- ISceneTreeAccessor: Свойства узла ---
		void SetActive(NodeHandle handle, bool active) override
		{
			ensure(IsValid(handle), "SceneTreeContainer::SetActive: невалидный NodeHandle");
			m_Nodes.metadata[handle.index].isActive = active;
		}

		[[nodiscard]] bool IsActive(NodeHandle handle) const override
		{
			ensure(IsValid(handle), "SceneTreeContainer::IsActive: невалидный NodeHandle");
			return m_Nodes.metadata[handle.index].isActive;
		}

		void SetSpatialHandle(NodeHandle handle, SpatialHandle spHandle) override
		{
			ensure(IsValid(handle), "SceneTreeContainer::SetSpatialHandle: невалидный NodeHandle");
			m_Nodes.metadata[handle.index].spatialHandle = spHandle;
		}

		[[nodiscard]] SpatialHandle GetSpatialHandle(NodeHandle handle) const noexcept override
		{
			ensure(IsValid(handle), "SceneTreeContainer::GetSpatialHandle: невалидный NodeHandle");
			return m_Nodes.metadata[handle.index].spatialHandle;
		}

		// --- Валидация ---
		[[nodiscard]] bool IsValid(NodeHandle handle) const noexcept
		{
			return handle.index < m_Nodes.metadata.size()
				&& m_Nodes.metadata[handle.index].isAlive
				&& m_Nodes.metadata[handle.index].generation == handle.generation;
		}

		[[nodiscard]] const NodeStorageBlock& GetNodes() const noexcept { return m_Nodes; }
#pragma endregion

	private:
		NodeStorageBlock m_Nodes;
		std::vector<uint32_t> m_FreeIndices;
	};
}
