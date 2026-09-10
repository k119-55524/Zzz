
#include "core/utils/Ensure.h"

#include "engine/scene/gameobject/GameObject.h"

#include "SceneTreeContainer.h"

using namespace zzz::core;

namespace zzz::engine
{
	SceneTreeContainer::SceneTreeContainer()
		: m_Nodes(1)
	{
	}

	NodeHandle SceneTreeContainer::CreateNode(GameObject* owner)
	{
		uint32_t index = 0xFFFFFFFF;
		if (!m_FreeIndices.empty())
		{
			index = m_FreeIndices.back();
			m_FreeIndices.pop_back();

			auto& meta = m_Nodes.metadata[index];
			meta.payloadType = owner ? SceneNodeType::GameObject : SceneNodeType::Empty;
			meta.gameObject = owner;
			meta.spatialHandle = 0xFFFFFFFF;
			meta.isAlive = true;
			meta.isActive = true;

			m_Nodes.topology[index] = NodeTopology{};
			m_Nodes.localTransforms[index] = LocalTransform{};
			m_Nodes.localMatrices[index] = Mat4<zF32>::Identity();
			m_Nodes.worldMatrices[index] = Mat4<zF32>::Identity();

			m_Nodes.MarkDirty(index);

			NodeHandle handle{ index, meta.generation };
			if (owner != nullptr)
			{
				owner->BindSceneTree(this, handle);
			}

			return handle;
		}

		index = static_cast<uint32_t>(m_Nodes.metadata.size());

		m_Nodes.topology.push_back(NodeTopology{});
		m_Nodes.localTransforms.push_back(LocalTransform{});
		m_Nodes.localMatrices.push_back(Mat4<zF32>::Identity());
		m_Nodes.worldMatrices.push_back(Mat4<zF32>::Identity());

		NodeMetadata meta;
		meta.payloadType = owner ? SceneNodeType::GameObject : SceneNodeType::Empty;
		meta.gameObject = owner;
		meta.spatialHandle = 0xFFFFFFFF;
		meta.generation = 1;
		meta.isAlive = true;
		meta.isActive = true;
		m_Nodes.metadata.push_back(std::move(meta));

		m_Nodes.EnsureCapacity(m_Nodes.metadata.size());
		m_Nodes.MarkDirty(index);

		NodeHandle handle{ index, 1 };
		if (owner != nullptr)
		{
			owner->BindSceneTree(this, handle);
		}

		return handle;
	}

	void SceneTreeContainer::SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform)
	{
		ensure(IsValid(child), "SceneTreeContainer::SetParent: child невалиден");
		if (child == parent)
		{
			return;
		}

		const uint32_t childIdx = child.index;
		const uint32_t newParentIdx = parent.IsValid() ? parent.index : 0xFFFFFFFF;

		auto& childTopo = m_Nodes.topology[childIdx];
		const uint32_t oldParentIdx = childTopo.parentIndex;
		if (oldParentIdx == newParentIdx)
		{
			return;
		}

		Mat4<zF32> oldWorld = m_Nodes.worldMatrices[childIdx];

		// 1. Отвязать от старого родителя
		if (oldParentIdx != 0xFFFFFFFF && oldParentIdx < m_Nodes.metadata.size())
		{
			const uint32_t prev = childTopo.prevSiblingIndex;
			const uint32_t next = childTopo.nextSiblingIndex;

			if (prev != 0xFFFFFFFF && prev < m_Nodes.metadata.size())
			{
				m_Nodes.topology[prev].nextSiblingIndex = next;
			}
			else if (m_Nodes.topology[oldParentIdx].firstChildIndex == childIdx)
			{
				m_Nodes.topology[oldParentIdx].firstChildIndex = next;
			}

			if (next != 0xFFFFFFFF && next < m_Nodes.metadata.size())
			{
				m_Nodes.topology[next].prevSiblingIndex = prev;
			}
		}

		// 2. Привязать к новому родителю
		childTopo.parentIndex = newParentIdx;
		childTopo.prevSiblingIndex = 0xFFFFFFFF;
		childTopo.nextSiblingIndex = 0xFFFFFFFF;

		if (newParentIdx != 0xFFFFFFFF && newParentIdx < m_Nodes.metadata.size())
		{
			const uint32_t oldFirst = m_Nodes.topology[newParentIdx].firstChildIndex;
			childTopo.nextSiblingIndex = oldFirst;
			if (oldFirst != 0xFFFFFFFF && oldFirst < m_Nodes.metadata.size())
			{
				m_Nodes.topology[oldFirst].prevSiblingIndex = childIdx;
			}
			m_Nodes.topology[newParentIdx].firstChildIndex = childIdx;
		}

		// 3. Сохранение мирового положения при смене родителя
		if (keepWorldTransform)
		{
			Mat4<zF32> newLocalMat = oldWorld;
			if (newParentIdx != 0xFFFFFFFF)
			{
				const auto invParent = m_Nodes.worldMatrices[newParentIdx].Inverse();
				newLocalMat = oldWorld * invParent;
			}

			const Vec3<zF32> newPos{ newLocalMat._41, newLocalMat._42, newLocalMat._43 };
			const Vec3<zF32> row0{ newLocalMat._11, newLocalMat._12, newLocalMat._13 };
			const Vec3<zF32> row1{ newLocalMat._21, newLocalMat._22, newLocalMat._23 };
			const Vec3<zF32> row2{ newLocalMat._31, newLocalMat._32, newLocalMat._33 };
			const Vec3<zF32> newScale{ row0.Length(), row1.Length(), row2.Length() };

			Mat3<zF32> rotMat{
				newScale.x > 1e-6f ? row0.x / newScale.x : 1.0f, newScale.x > 1e-6f ? row0.y / newScale.x : 0.0f, newScale.x > 1e-6f ? row0.z / newScale.x : 0.0f,
				newScale.y > 1e-6f ? row1.x / newScale.y : 0.0f, newScale.y > 1e-6f ? row1.y / newScale.y : 1.0f, newScale.y > 1e-6f ? row1.z / newScale.y : 0.0f,
				newScale.z > 1e-6f ? row2.x / newScale.z : 0.0f, newScale.z > 1e-6f ? row2.y / newScale.z : 0.0f, newScale.z > 1e-6f ? row2.z / newScale.z : 1.0f
			};

			m_Nodes.localTransforms[childIdx].position = newPos;
			m_Nodes.localTransforms[childIdx].rotation = Quat<zF32>::FromRotationMatrix(rotMat);
			m_Nodes.localTransforms[childIdx].scale = newScale;
		}

		m_Nodes.MarkDirty(childIdx);
	}

	void SceneTreeContainer::BeginFrame()
	{
		m_Nodes.dirtyTracker.Prepare(static_cast<uint32_t>(m_Nodes.metadata.size()));
	}

	void SceneTreeContainer::ResolveTransforms()
	{
		m_Nodes.ResolveTransforms();
	}
}
