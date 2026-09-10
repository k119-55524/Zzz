#pragma once

#include <vector>
#include <cstdint>
#include "core/utils/Ensure.h"
#include "core/utils/macros/MiscMacros.h"
#include "core/containers/BitTreeTracker.h"
#include "engine/scene/storage/ISceneTreeAccessor.h"

using namespace zzz::core;
using namespace zzz::math;

namespace zzz::engine
{
	/**
	 * @class NodeStorageBlock
	 * @brief Плоский SoA-блок (Structure of Arrays) данных узлов сцены.
	 *
	 * @details Хранит параллельные непрерывные векторы топологии, локальных TRS,
	 * локальных и мировых матриц, метаданных узлов и битовое дерево изменений (BitTreeTracker).
	 */
	class NodeStorageBlock final
	{
	public:
		std::vector<NodeTopology>   topology;
		std::vector<LocalTransform> localTransforms;
		std::vector<Mat4<zF32>>     localMatrices;
		std::vector<Mat4<zF32>>     worldMatrices;
		std::vector<NodeMetadata>   metadata;
		BitTreeTracker              dirtyTracker;

		explicit NodeStorageBlock(uint32_t initialCapacity = 1);
		~NodeStorageBlock() = default;

		Z_NO_COPY_MOVE(NodeStorageBlock);

		void EnsureCapacity(size_t requiredCapacity);
		inline void MarkDirty(uint32_t nodeIndex)
		{
			ensure(nodeIndex < metadata.size(), "NodeStorageBlock::MarkDirty: индекс узла выходит за пределы metadata");
			dirtyTracker.Set(nodeIndex);
		}

		/// @brief Пакетный расчёт локальных и мировых матриц по дереву.
		/// @details Обходит изменившиеся узлы из dirtyTracker. Если узел dirty, его localMatrix
		/// пересчитывается из TRS, затем обновляется worldMatrix и каскадно спускается детям.
		/// Для чистых детей переиспользуется их готовая localMatrix.
		/// @todo Подумать, как при обходе dirty битовой маски исключить повторный пересчёт матриц
		/// (например, если родитель уже обновил всё поддерево детей каскадом, а ребёнок тоже был в dirtyTracker,
		/// либо через топологическую сортировку/глубину, либо снимая dirty-бит у посещённых потомков).
		void ResolveTransforms();

	private:
		void ResolveSubtree(uint32_t nodeIndex, const Mat4<zF32>& parentWorld);
	};
}
