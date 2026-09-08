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
			metadata[nodeIndex].isDirty = true;
			dirtyTracker.Set(nodeIndex);
		}
	}

	void NodeStorageBlock::ResolveTransforms()
	{
		const size_t count = topology.size();
		if (count == 0)
		{
			return;
		}

		const math::Mat4<zF32> identity = math::Mat4<zF32>::Identity();

		// Итерируемся по корням дерева (у кого parentIndex == 0xFFFFFFFF)
		for (uint32_t i = 0; i < static_cast<uint32_t>(count); ++i)
		{
			if (metadata[i].isAlive && topology[i].parentIndex == 0xFFFFFFFF)
			{
				ResolveSubtree(i, false, identity);
			}
		}
	}

	void NodeStorageBlock::ResolveSubtree(uint32_t nodeIndex, bool parentDirty, const math::Mat4<zF32>& parentWorld)
	{
		auto& meta = metadata[nodeIndex];
		if (!meta.isAlive || !meta.isActive)
		{
			return;
		}

		const bool isSelfDirty = meta.isDirty;
		const bool needRecalc = parentDirty || isSelfDirty;

		if (needRecalc)
		{
			const auto& local = localTransforms[nodeIndex];
			const auto localMatrix = math::Mat4<zF32>::Scaling(local.scale) * local.rotation.ToMat4() * math::Mat4<zF32>::Translation(local.position);

			if (topology[nodeIndex].parentIndex != 0xFFFFFFFF)
			{
				worldMatrices[nodeIndex] = localMatrix * parentWorld;
			}
			else
			{
				worldMatrices[nodeIndex] = localMatrix;
			}

			meta.isDirty = false;
			dirtyTracker.Set(nodeIndex);
		}

		const auto& currentWorld = worldMatrices[nodeIndex];

		// Рекурсивно (или по сиблингам) обходим всех детей
		uint32_t childIndex = topology[nodeIndex].firstChildIndex;
		while (childIndex != 0xFFFFFFFF && childIndex < metadata.size())
		{
			ResolveSubtree(childIndex, needRecalc, currentWorld);
			childIndex = topology[childIndex].nextSiblingIndex;
		}
	}
}
