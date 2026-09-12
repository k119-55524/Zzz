
#include "core/utils/Ensure.h"

#include "engine/scene/gameobject/GameObject.h"
#include "core/io/package/GameObjectData.h"

#include "NodeStorage.h"

using namespace zzz::core;

namespace zzz::engine
{
	NodeStorage::NodeStorage()
		: m_DirtyTracker(1)
	{
	}

	NodeStorage::NodeStorage(std::span<const zzz::core::GameObjectData> objects)
		: m_DirtyTracker(static_cast<uint32_t>(objects.empty() ? 1 : objects.size()))
	{
		const size_t count = objects.size();
		if (count == 0)
		{
			return;
		}

		// 1. Выделяем память под все узлы одним махом
		m_Topology.resize(count);
		m_LocalTransforms.resize(count);
		m_LocalMatrices.resize(count);
		m_WorldMatrices.resize(count);
		m_Metadata.resize(count);

		m_DirtyTracker.Prepare(static_cast<uint32_t>(count));

		// 2. Заполняем плоские массивы узлов
		for (size_t i = 0; i < count; ++i)
		{
			const auto& obj = objects[i];

			m_Topology[i] = NodeTopology{};

			m_LocalTransforms[i].position = obj.GetPosition();
			m_LocalTransforms[i].rotation = obj.GetRotation();
			m_LocalTransforms[i].scale = obj.GetScale();

			m_LocalMatrices[i] = Mat4<zF32>::Identity();
			m_WorldMatrices[i] = Mat4<zF32>::Identity();

			auto& meta = m_Metadata[i];
			meta.payloadType = obj.IsEntity() ? SceneNodeType::Entity : SceneNodeType::GameObject;
			meta.gameObject = nullptr;
			meta.spatialHandle = static_cast<SpatialHandle>(i);
			meta.layerObjectIndex = static_cast<uint32_t>(i);
			meta.generation = 1;
			meta.isAlive = true;
			meta.isActive = obj.IsActive();

			m_DirtyTracker.Set(static_cast<uint32_t>(i));
		}

		// 3. Выстраиваем связи иерархии parent -> child
		for (size_t i = 0; i < count; ++i)
		{
			const uint32_t pIdx = objects[i].GetParentIndex();
			if (pIdx != 0xFFFFFFFF && pIdx < count)
			{
				const uint32_t childIdx = static_cast<uint32_t>(i);
				m_Topology[childIdx].parentIndex = pIdx;

				const uint32_t oldFirst = m_Topology[pIdx].firstChildIndex;
				m_Topology[childIdx].nextSiblingIndex = oldFirst;
				if (oldFirst != 0xFFFFFFFF && oldFirst < count)
				{
					m_Topology[oldFirst].prevSiblingIndex = childIdx;
				}
				m_Topology[pIdx].firstChildIndex = childIdx;
			}
		}

		// 4. Сразу рассчитываем матрицы для всех узлов
		ResolveTransforms();
	}

	void NodeStorage::SetParent(NodeHandle child, NodeHandle parent, bool keepWorldTransform)
	{
		ensure(IsValid(child), "NodeStorage::SetParent: child невалиден");
		if (child == parent)
		{
			return;
		}

		const uint32_t childIdx = child.index;
		const uint32_t newParentIdx = parent.IsValid() ? parent.index : 0xFFFFFFFF;

		auto& childTopo = m_Topology[childIdx];
		const uint32_t oldParentIdx = childTopo.parentIndex;
		if (oldParentIdx == newParentIdx)
		{
			return;
		}

		Mat4<zF32> oldWorld = m_WorldMatrices[childIdx];

		// 1. Отвязать от старого родителя
		if (oldParentIdx != 0xFFFFFFFF && oldParentIdx < m_Metadata.size())
		{
			const uint32_t prev = childTopo.prevSiblingIndex;
			const uint32_t next = childTopo.nextSiblingIndex;

			if (prev != 0xFFFFFFFF && prev < m_Metadata.size())
			{
				m_Topology[prev].nextSiblingIndex = next;
			}
			else if (m_Topology[oldParentIdx].firstChildIndex == childIdx)
			{
				m_Topology[oldParentIdx].firstChildIndex = next;
			}

			if (next != 0xFFFFFFFF && next < m_Metadata.size())
			{
				m_Topology[next].prevSiblingIndex = prev;
			}
		}

		// 2. Привязать к новому родителю
		childTopo.parentIndex = newParentIdx;
		childTopo.prevSiblingIndex = 0xFFFFFFFF;
		childTopo.nextSiblingIndex = 0xFFFFFFFF;

		if (newParentIdx != 0xFFFFFFFF && newParentIdx < m_Metadata.size())
		{
			const uint32_t oldFirst = m_Topology[newParentIdx].firstChildIndex;
			childTopo.nextSiblingIndex = oldFirst;
			if (oldFirst != 0xFFFFFFFF && oldFirst < m_Metadata.size())
			{
				m_Topology[oldFirst].prevSiblingIndex = childIdx;
			}
			m_Topology[newParentIdx].firstChildIndex = childIdx;
		}

		// 3. Сохранение мирового положения при смене родителя
		if (keepWorldTransform)
		{
			Mat4<zF32> newLocalMat = oldWorld;
			if (newParentIdx != 0xFFFFFFFF)
			{
				const auto invParent = m_WorldMatrices[newParentIdx].Inverse();
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

			m_LocalTransforms[childIdx].position = newPos;
			m_LocalTransforms[childIdx].rotation = Quat<zF32>::FromRotationMatrix(rotMat);
			m_LocalTransforms[childIdx].scale = newScale;
		}

		MarkDirty(childIdx);
	}

	void NodeStorage::BeginFrame()
	{
		m_DirtyTracker.Prepare(static_cast<uint32_t>(m_Metadata.size()));
	}

	void NodeStorage::ResolveTransforms()
	{
		const auto dirtyIndices = m_DirtyTracker.GetDirtyIndices();
		if (dirtyIndices.empty())
		{
			return;
		}

		for (const uint32_t nodeIndex : dirtyIndices)
		{
			if (nodeIndex >= m_Metadata.size())
			{
				continue;
			}

			const auto& meta = m_Metadata[nodeIndex];
			if (!meta.isAlive || !meta.isActive)
			{
				continue;
			}

			const auto& local = m_LocalTransforms[nodeIndex];
			m_LocalMatrices[nodeIndex] = Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);

			const uint32_t parentIdx = m_Topology[nodeIndex].parentIndex;
			if (parentIdx != 0xFFFFFFFF && parentIdx < m_Metadata.size())
			{
				m_WorldMatrices[nodeIndex] = m_LocalMatrices[nodeIndex] * m_WorldMatrices[parentIdx];
			}
			else
			{
				m_WorldMatrices[nodeIndex] = m_LocalMatrices[nodeIndex];
			}

			// Каскадно обновляем детей (используя готовую m_LocalMatrices[child], если сам ребёнок не dirty)
			const uint32_t firstChild = m_Topology[nodeIndex].firstChildIndex;
			if (firstChild != 0xFFFFFFFF && firstChild < m_Metadata.size())
			{
				const auto& currentWorld = m_WorldMatrices[nodeIndex];
				uint32_t childIndex = firstChild;
				while (childIndex != 0xFFFFFFFF && childIndex < m_Metadata.size())
				{
					ResolveSubtree(childIndex, currentWorld);
					childIndex = m_Topology[childIndex].nextSiblingIndex;
				}
			}
		}
	}

	void NodeStorage::ResolveSubtree(uint32_t nodeIndex, const Mat4<zF32>& parentWorld)
	{
		const auto& meta = m_Metadata[nodeIndex];
		if (!meta.isAlive || !meta.isActive)
		{
			return;
		}

		// Если сам ребёнок был помечен как dirty, его TRS изменился — обновляем localMatrix
		if (m_DirtyTracker.IsSet(nodeIndex))
		{
			const auto& local = m_LocalTransforms[nodeIndex];
			m_LocalMatrices[nodeIndex] = Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);
		}

		// Вычисляем мировую матрицу ребенка: (его локальная * родительский мир)
		m_WorldMatrices[nodeIndex] = m_LocalMatrices[nodeIndex] * parentWorld;

		const auto& currentWorld = m_WorldMatrices[nodeIndex];
		uint32_t childIndex = m_Topology[nodeIndex].firstChildIndex;
		while (childIndex != 0xFFFFFFFF && childIndex < m_Metadata.size())
		{
			ResolveSubtree(childIndex, currentWorld);
			childIndex = m_Topology[childIndex].nextSiblingIndex;
		}
	}
}

