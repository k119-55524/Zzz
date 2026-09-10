
#include <algorithm>

#include "engine/scene/gameobject/GameObject.h"
#include "engine/scene/storage/ISpatialStorage.h"

#include "SceneTreeContainer.h"

namespace zzz::engine
{
	namespace
	{
		const math::Vec3<zF32> kDefaultPos{ 0.0f, 0.0f, 0.0f };
		const math::Quat<zF32> kDefaultRot{ 0.0f, 0.0f, 0.0f, 1.0f };
		const math::Vec3<zF32> kDefaultScale{ 1.0f, 1.0f, 1.0f };
		const math::Mat4<zF32> kIdentityMatrix = math::Mat4<zF32>::Identity();
		const std::string kEmptyString{};
	}

	SceneTreeContainer::SceneTreeContainer()
		: m_PrimaryNodes(1)
		, m_SecondaryNodes(1)
	{
	}

	NodeHandle SceneTreeContainer::CreateNode(std::string name, GameObject* owner)
	{
		uint32_t index = 0xFFFFFFFF;
		if (!m_FreeIndices.empty())
		{
			index = m_FreeIndices.back();
			m_FreeIndices.pop_back();

			auto& meta = m_SecondaryNodes.metadata[index];
			meta.name = std::move(name);
			meta.payloadType = owner ? SceneNodeType::GameObject : SceneNodeType::Empty;
			meta.gameObject = owner;
			meta.spatialHandle = 0xFFFFFFFF;
			meta.isAlive = true;
			meta.isActive = true;
			meta.isDirty = true;

			m_SecondaryNodes.topology[index] = NodeTopology{};
			m_SecondaryNodes.localTransforms[index] = LocalTransform{};
			m_SecondaryNodes.worldMatrices[index] = kIdentityMatrix;

			m_SecondaryNodes.dirtyTracker.Set(index);
			m_TopologyDirty = true;

			NodeHandle handle{ index, meta.generation };
			if (owner != nullptr)
			{
				owner->BindSceneTree(this, handle);
			}

			return handle;
		}

		index = static_cast<uint32_t>(m_SecondaryNodes.metadata.size());

		m_SecondaryNodes.topology.push_back(NodeTopology{});
		m_SecondaryNodes.localTransforms.push_back(LocalTransform{});
		m_SecondaryNodes.worldMatrices.push_back(kIdentityMatrix);

		NodeMetadata meta;
		meta.name = std::move(name);
		meta.payloadType = owner ? SceneNodeType::GameObject : SceneNodeType::Empty;
		meta.gameObject = owner;
		meta.spatialHandle = 0xFFFFFFFF;
		meta.generation = 1;
		meta.isAlive = true;
		meta.isActive = true;
		meta.isDirty = true;
		m_SecondaryNodes.metadata.push_back(std::move(meta));

		m_SecondaryNodes.EnsureCapacity(m_SecondaryNodes.metadata.size());
		m_SecondaryNodes.dirtyTracker.Set(index);
		m_TopologyDirty = true;

		NodeHandle handle{ index, 1 };
		if (owner != nullptr)
		{
			owner->BindSceneTree(this, handle);
		}

		return handle;
	}

	void SceneTreeContainer::DestroySubtree(NodeHandle root, ISpatialStorage* spatialStorage)
	{
		if (!IsValid(root))
		{
			return;
		}

		// Итеративный сбор индексов поддерева (BFS)
		std::vector<uint32_t> toDelete;
		toDelete.reserve(16);
		toDelete.push_back(root.index);

		size_t readIdx = 0;
		while (readIdx < toDelete.size())
		{
			const uint32_t cur = toDelete[readIdx++];
			uint32_t child = m_SecondaryNodes.topology[cur].firstChildIndex;
			while (child != 0xFFFFFFFF && child < m_SecondaryNodes.metadata.size())
			{
				toDelete.push_back(child);
				child = m_SecondaryNodes.topology[child].nextSiblingIndex;
			}
		}

		// Отвязка корня поддерева от родительской топологии
		const uint32_t rootIdx = root.index;
		const uint32_t parentIdx = m_SecondaryNodes.topology[rootIdx].parentIndex;
		if (parentIdx != 0xFFFFFFFF && parentIdx < m_SecondaryNodes.metadata.size())
		{
			const uint32_t prev = m_SecondaryNodes.topology[rootIdx].prevSiblingIndex;
			const uint32_t next = m_SecondaryNodes.topology[rootIdx].nextSiblingIndex;

			if (prev != 0xFFFFFFFF && prev < m_SecondaryNodes.metadata.size())
			{
				m_SecondaryNodes.topology[prev].nextSiblingIndex = next;
			}
			else if (m_SecondaryNodes.topology[parentIdx].firstChildIndex == rootIdx)
			{
				m_SecondaryNodes.topology[parentIdx].firstChildIndex = next;
			}

			if (next != 0xFFFFFFFF && next < m_SecondaryNodes.metadata.size())
			{
				m_SecondaryNodes.topology[next].prevSiblingIndex = prev;
			}
		}

		// Уничтожение узлов и освобождение слотов
		for (uint32_t idx : toDelete)
		{
			auto& meta = m_SecondaryNodes.metadata[idx];
			if (spatialStorage != nullptr && meta.spatialHandle != 0xFFFFFFFF)
			{
				spatialStorage->Remove(meta.spatialHandle);
				meta.spatialHandle = 0xFFFFFFFF;
			}

			meta.gameObject = nullptr;
			meta.isAlive = false;
			meta.isDirty = false;
			meta.generation++;

			m_FreeIndices.push_back(idx);
			m_SecondaryNodes.topology[idx] = NodeTopology{};
			m_SecondaryNodes.dirtyTracker.Set(idx);
		}

		m_TopologyDirty = true;
	}

	void SceneTreeContainer::SetLocalPosition(NodeHandle handle, const ::zzz::math::Vec3<zF32>& pos)
	{
		if (IsValid(handle))
		{
			m_SecondaryNodes.localTransforms[handle.index].position = pos;
			m_SecondaryNodes.MarkDirty(handle.index);
		}
	}

	const ::zzz::math::Vec3<zF32>& SceneTreeContainer::GetLocalPosition(NodeHandle handle) const
	{
		if (IsValid(handle))
		{
			return m_SecondaryNodes.localTransforms[handle.index].position;
		}
		return kDefaultPos;
	}

	void SceneTreeContainer::SetLocalRotation(NodeHandle handle, const ::zzz::math::Quat<zF32>& rot)
	{
		if (IsValid(handle))
		{
			m_SecondaryNodes.localTransforms[handle.index].rotation = rot;
			m_SecondaryNodes.MarkDirty(handle.index);
		}
	}

	const ::zzz::math::Quat<zF32>& SceneTreeContainer::GetLocalRotation(NodeHandle handle) const
	{
		if (IsValid(handle))
		{
			return m_SecondaryNodes.localTransforms[handle.index].rotation;
		}
		return kDefaultRot;
	}

	void SceneTreeContainer::SetLocalScale(NodeHandle handle, const ::zzz::math::Vec3<zF32>& scale)
	{
		if (IsValid(handle))
		{
			m_SecondaryNodes.localTransforms[handle.index].scale = scale;
			m_SecondaryNodes.MarkDirty(handle.index);
		}
	}

	const ::zzz::math::Vec3<zF32>& SceneTreeContainer::GetLocalScale(NodeHandle handle) const
	{
		if (IsValid(handle))
		{
			return m_SecondaryNodes.localTransforms[handle.index].scale;
		}
		return kDefaultScale;
	}

	const ::zzz::math::Mat4<zF32>& SceneTreeContainer::GetWorldMatrix(NodeHandle handle) const
	{
		if (IsValid(handle))
		{
			return m_SecondaryNodes.worldMatrices[handle.index];
		}
		return kIdentityMatrix;
	}

	void SceneTreeContainer::SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform)
	{
		if (!IsValid(child) || child == parent)
		{
			return;
		}

		const uint32_t childIdx = child.index;
		const uint32_t newParentIdx = parent.IsValid() ? parent.index : 0xFFFFFFFF;

		auto& childTopo = m_SecondaryNodes.topology[childIdx];
		const uint32_t oldParentIdx = childTopo.parentIndex;
		if (oldParentIdx == newParentIdx)
		{
			return;
		}

		math::Mat4<zF32> oldWorld = m_SecondaryNodes.worldMatrices[childIdx];

		// 1. Отвязать от старого родителя
		if (oldParentIdx != 0xFFFFFFFF && oldParentIdx < m_SecondaryNodes.metadata.size())
		{
			const uint32_t prev = childTopo.prevSiblingIndex;
			const uint32_t next = childTopo.nextSiblingIndex;

			if (prev != 0xFFFFFFFF && prev < m_SecondaryNodes.metadata.size())
			{
				m_SecondaryNodes.topology[prev].nextSiblingIndex = next;
			}
			else if (m_SecondaryNodes.topology[oldParentIdx].firstChildIndex == childIdx)
			{
				m_SecondaryNodes.topology[oldParentIdx].firstChildIndex = next;
			}

			if (next != 0xFFFFFFFF && next < m_SecondaryNodes.metadata.size())
			{
				m_SecondaryNodes.topology[next].prevSiblingIndex = prev;
			}
		}

		// 2. Привязать к новому родителю
		childTopo.parentIndex = newParentIdx;
		childTopo.prevSiblingIndex = 0xFFFFFFFF;
		childTopo.nextSiblingIndex = 0xFFFFFFFF;

		if (newParentIdx != 0xFFFFFFFF && newParentIdx < m_SecondaryNodes.metadata.size())
		{
			const uint32_t oldFirst = m_SecondaryNodes.topology[newParentIdx].firstChildIndex;
			childTopo.nextSiblingIndex = oldFirst;
			if (oldFirst != 0xFFFFFFFF && oldFirst < m_SecondaryNodes.metadata.size())
			{
				m_SecondaryNodes.topology[oldFirst].prevSiblingIndex = childIdx;
			}
			m_SecondaryNodes.topology[newParentIdx].firstChildIndex = childIdx;
		}

		// 3. Сохранение мирового положения при смене родителя
		if (keepWorldTransform)
		{
			math::Mat4<zF32> newLocalMat = oldWorld;
			if (newParentIdx != 0xFFFFFFFF)
			{
				const auto invParent = m_SecondaryNodes.worldMatrices[newParentIdx].Inverse();
				newLocalMat = oldWorld * invParent;
			}

			const math::Vec3<zF32> newPos{ newLocalMat._41, newLocalMat._42, newLocalMat._43 };
			const math::Vec3<zF32> row0{ newLocalMat._11, newLocalMat._12, newLocalMat._13 };
			const math::Vec3<zF32> row1{ newLocalMat._21, newLocalMat._22, newLocalMat._23 };
			const math::Vec3<zF32> row2{ newLocalMat._31, newLocalMat._32, newLocalMat._33 };
			const math::Vec3<zF32> newScale{ row0.Length(), row1.Length(), row2.Length() };

			math::Mat3<zF32> rotMat{
				newScale.x > 1e-6f ? row0.x / newScale.x : 1.0f, newScale.x > 1e-6f ? row0.y / newScale.x : 0.0f, newScale.x > 1e-6f ? row0.z / newScale.x : 0.0f,
				newScale.y > 1e-6f ? row1.x / newScale.y : 0.0f, newScale.y > 1e-6f ? row1.y / newScale.y : 1.0f, newScale.y > 1e-6f ? row1.z / newScale.y : 0.0f,
				newScale.z > 1e-6f ? row2.x / newScale.z : 0.0f, newScale.z > 1e-6f ? row2.y / newScale.z : 0.0f, newScale.z > 1e-6f ? row2.z / newScale.z : 1.0f
			};

			m_SecondaryNodes.localTransforms[childIdx].position = newPos;
			m_SecondaryNodes.localTransforms[childIdx].rotation = math::Quat<zF32>::FromRotationMatrix(rotMat);
			m_SecondaryNodes.localTransforms[childIdx].scale = newScale;
		}

		m_SecondaryNodes.MarkDirty(childIdx);
		m_TopologyDirty = true;
	}

	NodeHandle SceneTreeContainer::GetParent(NodeHandle handle) const
	{
		if (IsValid(handle))
		{
			const uint32_t pIdx = m_SecondaryNodes.topology[handle.index].parentIndex;
			if (pIdx != 0xFFFFFFFF && pIdx < m_SecondaryNodes.metadata.size())
			{
				return NodeHandle{ pIdx, m_SecondaryNodes.metadata[pIdx].generation };
			}
		}
		return NodeHandle{};
	}

	GameObject* SceneTreeContainer::GetNodeOwner(NodeHandle handle) const
	{
		if (IsValid(handle))
		{
			return m_SecondaryNodes.metadata[handle.index].gameObject;
		}
		return nullptr;
	}

	void SceneTreeContainer::MarkDirty(NodeHandle handle)
	{
		if (IsValid(handle))
		{
			m_SecondaryNodes.MarkDirty(handle.index);
		}
	}

	void SceneTreeContainer::SetActive(NodeHandle handle, bool active)
	{
		if (IsValid(handle))
		{
			m_SecondaryNodes.metadata[handle.index].isActive = active;
			m_SecondaryNodes.MarkDirty(handle.index);
		}
	}

	bool SceneTreeContainer::IsActive(NodeHandle handle) const
	{
		if (IsValid(handle))
		{
			return m_SecondaryNodes.metadata[handle.index].isActive;
		}
		return false;
	}

	void SceneTreeContainer::SetName(NodeHandle handle, std::string name)
	{
		if (IsValid(handle))
		{
			m_SecondaryNodes.metadata[handle.index].name = std::move(name);
		}
	}

	const std::string& SceneTreeContainer::GetName(NodeHandle handle) const
	{
		if (IsValid(handle))
		{
			return m_SecondaryNodes.metadata[handle.index].name;
		}
		return kEmptyString;
	}

	void SceneTreeContainer::SetSpatialHandle(NodeHandle handle, SpatialHandle spHandle)
	{
		if (IsValid(handle))
		{
			m_SecondaryNodes.metadata[handle.index].spatialHandle = spHandle;
		}
	}

	SpatialHandle SceneTreeContainer::GetSpatialHandle(NodeHandle handle) const noexcept
	{
		if (IsValid(handle))
		{
			return m_SecondaryNodes.metadata[handle.index].spatialHandle;
		}
		return 0xFFFFFFFF;
	}

	bool SceneTreeContainer::IsValid(NodeHandle handle) const noexcept
	{
		return handle.index < m_SecondaryNodes.metadata.size()
			&& m_SecondaryNodes.metadata[handle.index].isAlive
			&& m_SecondaryNodes.metadata[handle.index].generation == handle.generation;
	}

	void SceneTreeContainer::BeginFrame()
	{
		m_SecondaryNodes.dirtyTracker.Prepare(static_cast<uint32_t>(m_SecondaryNodes.metadata.size()));
	}

	void SceneTreeContainer::ResolveTransforms()
	{
		m_SecondaryNodes.ResolveTransforms();
	}

	void SceneTreeContainer::ApplyHandoverBarrier()
	{
		const size_t secSize = m_SecondaryNodes.metadata.size();

		// 1. Приведение размеров буфера рендера к размеру логики
		if (m_PrimaryNodes.metadata.size() != secSize)
		{
			m_PrimaryNodes.topology.resize(secSize);
			m_PrimaryNodes.localTransforms.resize(secSize);
			m_PrimaryNodes.worldMatrices.resize(secSize);
			m_PrimaryNodes.metadata.resize(secSize);
			m_TopologyDirty = true;
		}

		// 2. Если менялась топология (создание, удаление, reparenting) — синхронизируем топологию
		if (m_TopologyDirty)
		{
			m_PrimaryNodes.topology = m_SecondaryNodes.topology;
			m_TopologyDirty = false;
		}

		// 3. Дифференциальное обновление изменённых слотов по dirtyTracker
		auto dirtyIndices = m_SecondaryNodes.dirtyTracker.GetDirtyIndices();
		for (uint32_t idx : dirtyIndices)
		{
			if (idx < secSize)
			{
				m_PrimaryNodes.worldMatrices[idx] = m_SecondaryNodes.worldMatrices[idx];
				m_PrimaryNodes.localTransforms[idx] = m_SecondaryNodes.localTransforms[idx];

				auto& dstMeta = m_PrimaryNodes.metadata[idx];
				const auto& srcMeta = m_SecondaryNodes.metadata[idx];
				dstMeta.isAlive = srcMeta.isAlive;
				dstMeta.isActive = srcMeta.isActive;
				dstMeta.generation = srcMeta.generation;
				dstMeta.payloadType = srcMeta.payloadType;
				dstMeta.gameObject = srcMeta.gameObject;
				dstMeta.spatialHandle = srcMeta.spatialHandle;
				dstMeta.isDirty = srcMeta.isDirty;
				if (dstMeta.name != srcMeta.name)
				{
					dstMeta.name = srcMeta.name;
				}
			}
		}
	}
}
