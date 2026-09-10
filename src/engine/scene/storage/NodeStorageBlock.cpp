#include "engine/scene/storage/NodeStorageBlock.h"

namespace zzz::engine
{
	NodeStorageBlock::NodeStorageBlock(uint32_t initialCapacity)
		: dirtyTracker(initialCapacity)
	{
		EnsureCapacity(initialCapacity);
	}

	void NodeStorageBlock::EnsureCapacity(size_t requiredCapacity)
	{
		if (requiredCapacity == 0)
		{
			requiredCapacity = 1;
		}

		if (topology.capacity() < requiredCapacity)
		{
			topology.reserve(requiredCapacity);
			localTransforms.reserve(requiredCapacity);
			localMatrices.reserve(requiredCapacity);
			worldMatrices.reserve(requiredCapacity);
			metadata.reserve(requiredCapacity);
		}

		if (dirtyTracker.GetCapacity() < requiredCapacity)
		{
			dirtyTracker.GrowCapacity(static_cast<uint32_t>(requiredCapacity));
		}
	}

	void NodeStorageBlock::MarkDirty(uint32_t nodeIndex) noexcept
	{
		if (nodeIndex < metadata.size())
		{
			dirtyTracker.Set(nodeIndex);
		}
	}

	void NodeStorageBlock::ResolveTransforms()
	{
		const auto dirtyIndices = dirtyTracker.GetDirtyIndices();
		if (dirtyIndices.empty())
		{
			return;
		}

		// TODO: Подумать, как при обходе dirty битовой маски исключить повторный пересчёт матриц
		// (если предок уже каскадно обновил поддерево детей, потомки из dirtyIndices не должны пересчитываться повторно).
		for (const uint32_t nodeIndex : dirtyIndices)
		{
			if (nodeIndex >= metadata.size())
			{
				continue;
			}

			const auto& meta = metadata[nodeIndex];
			if (!meta.isAlive || !meta.isActive)
			{
				continue;
			}

			const auto& local = localTransforms[nodeIndex];
			localMatrices[nodeIndex] = Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);

			const uint32_t parentIdx = topology[nodeIndex].parentIndex;
			if (parentIdx != 0xFFFFFFFF && parentIdx < metadata.size())
			{
				worldMatrices[nodeIndex] = localMatrices[nodeIndex] * worldMatrices[parentIdx];
			}
			else
			{
				worldMatrices[nodeIndex] = localMatrices[nodeIndex];
			}

			// Каскадно обновляем детей (используя готовую localMatrices[child], если сам ребёнок не dirty)
			const uint32_t firstChild = topology[nodeIndex].firstChildIndex;
			if (firstChild != 0xFFFFFFFF && firstChild < metadata.size())
			{
				const auto& currentWorld = worldMatrices[nodeIndex];
				uint32_t childIndex = firstChild;
				while (childIndex != 0xFFFFFFFF && childIndex < metadata.size())
				{
					ResolveSubtree(childIndex, currentWorld);
					childIndex = topology[childIndex].nextSiblingIndex;
				}
			}
		}
	}

	void NodeStorageBlock::ResolveSubtree(uint32_t nodeIndex, const Mat4<zF32>& parentWorld)
	{
		const auto& meta = metadata[nodeIndex];
		if (!meta.isAlive || !meta.isActive)
		{
			return;
		}

		// Если сам ребёнок был помечен как dirty, его TRS изменился — обновляем localMatrix
		if (dirtyTracker.IsSet(nodeIndex))
		{
			const auto& local = localTransforms[nodeIndex];
			localMatrices[nodeIndex] = Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * Mat4<zF32>::Translation(local.position);
		}

		// Вычисляем мировую матрицу ребенка: (его локальная * родительский мир)
		worldMatrices[nodeIndex] = localMatrices[nodeIndex] * parentWorld;

		const auto& currentWorld = worldMatrices[nodeIndex];
		uint32_t childIndex = topology[nodeIndex].firstChildIndex;
		while (childIndex != 0xFFFFFFFF && childIndex < metadata.size())
		{
			ResolveSubtree(childIndex, currentWorld);
			childIndex = topology[childIndex].nextSiblingIndex;
		}
	}
}
