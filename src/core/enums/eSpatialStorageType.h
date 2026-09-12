#pragma once

#include <cstdint>
#include <string_view>

#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eSpatialStorageType
	 * @brief Тип пространственного индекса/хранилища слоя сцены.
	 */
	enum class eSpatialStorageType : uint8_t
	{
		Flat = 0,   ///< Базовый плоский список слотов (DefaultSpatialStorage)
		BVH = 1,    ///< Иерархия ограничивающих объемов (Bounding Volume Hierarchy)
		Octree = 2, ///< Восьмеричное дерево (для 3D)
		Grid = 3    ///< Пространственная сетка (Spatial Hashing / Uniform Grid)
	};

	[[nodiscard]] constexpr std::string_view ToString(eSpatialStorageType type)
	{
		switch (type)
		{
		case eSpatialStorageType::Flat:   return "Flat";
		case eSpatialStorageType::BVH:    return "BVH";
		case eSpatialStorageType::Octree: return "Octree";
		case eSpatialStorageType::Grid:   return "Grid";
		}
		THROW_RUNTIME("Необработанный eSpatialStorageType");
	}
}
