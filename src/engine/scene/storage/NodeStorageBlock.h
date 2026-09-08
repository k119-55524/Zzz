#pragma once

#include <vector>
#include <cstdint>
#include "core/utils/macros/MiscMacros.h"
#include "core/containers/BitTreeTracker.h"
#include "engine/scene/storage/ISceneTreeAccessor.h"

namespace zzz::engine
{
	/**
	 * @class NodeStorageBlock
	 * @brief Плоский SoA-блок (Structure of Arrays) данных узлов сцены.
	 *
	 * @details Хранит параллельные непрерывные векторы топологии, локальных и мировых трансформаций,
	 * метаданных и 64-арный трекер изменений.
	 */
	class NodeStorageBlock final
	{
	public:
		std::vector<NodeTopology>         topology;
		std::vector<LocalTransform>       localTransforms;
		std::vector<math::Mat4<zF32>>     worldMatrices;   // 64 байта = 1 кэш-линия
		std::vector<NodeMetadata>         metadata;
		core::BitTreeTracker              dirtyTracker;

		explicit NodeStorageBlock(uint32_t initialCapacity = 1);
		~NodeStorageBlock() = default;

		Z_NO_COPY_MOVE(NodeStorageBlock);

		void EnsureCapacity(size_t requiredCapacity);
		void MarkDirty(uint32_t nodeIndex) noexcept;

		/// @brief Пакетный расчёт мировых матриц (parent-before-child).
		/// @details Выполняет top-down обход по иерархии топологии от корней к листьям,
		/// каскадно распространяя пересчёт мировых матриц от изменившихся предков к потомкам.
		void ResolveTransforms();

	private:
		void ResolveSubtree(uint32_t nodeIndex, bool parentDirty, const math::Mat4<zF32>& parentWorld);
	};
}
