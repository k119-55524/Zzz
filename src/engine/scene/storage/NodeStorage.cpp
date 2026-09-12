
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
			return;

		// 1. Выделяем память под все узлы одним махом
		m_Topology.resize(count);
		m_LocalTransforms.resize(count);
		m_WorldMatrices.resize(count);
		m_Bindings.resize(count);

		m_DirtyTracker.Prepare(static_cast<uint32_t>(count));

		// 2. Заполняем плоские массивы узлов
		for (size_t i = 0; i < count; ++i)
		{
			const auto& obj = objects[i];

			m_Topology[i] = NodeTopology{
				.isActive = obj.IsActive(),
				.isStatic = false
			};

			m_LocalTransforms[i].position = obj.GetPosition();
			m_LocalTransforms[i].rotation = obj.GetRotation();
			m_LocalTransforms[i].scale = obj.GetScale();

			m_WorldMatrices[i] = Mat4<zF32>::Identity();

			m_Bindings[i] = NodeBindings{
				.spatialHandle = static_cast<zU32>(i),
				.layerObjectIndex = static_cast<zU32>(i)
			};

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

	void NodeStorage::BeginFrame()
	{
		m_DirtyTracker.Prepare(static_cast<uint32_t>(m_Topology.size()));
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
			if (nodeIndex >= m_Topology.size())
			{
				continue;
			}

			if (!m_Topology[nodeIndex].isActive)
			{
				continue;
			}

			const auto& local = m_LocalTransforms[nodeIndex];
			const Mat4<zF32> localMatrix = Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);

			const uint32_t parentIdx = m_Topology[nodeIndex].parentIndex;
			if (parentIdx != 0xFFFFFFFF && parentIdx < m_Topology.size())
			{
				m_WorldMatrices[nodeIndex] = localMatrix * m_WorldMatrices[parentIdx];
			}
			else
			{
				m_WorldMatrices[nodeIndex] = localMatrix;
			}

			// Каскадно обновляем детей
			const uint32_t firstChild = m_Topology[nodeIndex].firstChildIndex;
			if (firstChild != 0xFFFFFFFF && firstChild < m_Topology.size())
			{
				const auto& currentWorld = m_WorldMatrices[nodeIndex];
				uint32_t childIndex = firstChild;
				while (childIndex != 0xFFFFFFFF && childIndex < m_Topology.size())
				{
					ResolveSubtree(childIndex, currentWorld);
					childIndex = m_Topology[childIndex].nextSiblingIndex;
				}
			}
		}
	}

	void NodeStorage::ResolveSubtree(uint32_t nodeIndex, const Mat4<zF32>& parentWorld)
	{
		if (!m_Topology[nodeIndex].isActive)
		{
			return;
		}

		const auto& local = m_LocalTransforms[nodeIndex];
		const Mat4<zF32> localMatrix = Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);
		m_WorldMatrices[nodeIndex] = localMatrix * parentWorld;

		const auto& currentWorld = m_WorldMatrices[nodeIndex];
		uint32_t childIndex = m_Topology[nodeIndex].firstChildIndex;
		while (childIndex != 0xFFFFFFFF && childIndex < m_Topology.size())
		{
			ResolveSubtree(childIndex, currentWorld);
			childIndex = m_Topology[childIndex].nextSiblingIndex;
		}
	}
}

