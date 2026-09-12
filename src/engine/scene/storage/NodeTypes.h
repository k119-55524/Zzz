#pragma once

#include "math/quat/Quat.h"
#include "math/utils/Types.h"
#include "math/vector/Vec3.h"

using namespace zzz::math;

/**
 * @file NodeTypes.h
 * @brief Базовые типы данных и структуры узлов графа сцены (NodeStorage).
 */
 namespace zzz::engine
{
	inline constexpr zU32 kInvalidNodeIndex = 0xFFFFFFFF;

	/// @brief Топология иерархии сцены (First-Child / Next-Sibling) и флаги состояния узла.
	struct NodeTopology
	{
		zU32 parentIndex{ kInvalidNodeIndex };
		zU32 firstChildIndex{ kInvalidNodeIndex };
		zU32 nextSiblingIndex{ kInvalidNodeIndex };
		zU32 prevSiblingIndex{ kInvalidNodeIndex };

		bool isActive{ true };
		bool isStatic{ false };
	};

	/// @brief Компоненты пространственного положения (Pos, Rot, Scale).
	struct Transform
	{
		Vec3<zF32> position{ 0.0f, 0.0f, 0.0f };
		Quat<zF32> rotation{ 0.0f, 0.0f, 0.0f, 1.0f };
		Vec3<zF32> scale{ 1.0f, 1.0f, 1.0f };
	};

	/// @brief Внешние связи узла с другими подсистемами движка.
	struct NodeBindings
	{
		zU32 spatialHandle{ kInvalidNodeIndex };
		zU32 layerObjectIndex{ kInvalidNodeIndex };
	};
}
