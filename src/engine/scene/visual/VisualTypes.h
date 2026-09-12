#pragma once

#include <vector>
#include <variant>

#include "core/utils/Guid.h"
#include "math/utils/Types.h"
#include "core/enums/eVisualType.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @struct SimpleMeshData
	 * @brief Меш (сетка + материал, для SimpleMesh3D и Mesh2D).
	 */
	struct SimpleMeshData
	{
		Guid meshGuid;
		Guid materialGuid;
	};

	/**
	 * @struct MultiMesh3DData
	 * @brief Составной 3D-меш (N сабмешей + N материалов по индексу).
	 */
	struct MultiMesh3DData
	{
		std::vector<Guid> submeshes;
		std::vector<Guid> materials;
	};

	/**
	 * @struct SpriteData
	 * @brief 2D-спрайт с текстурой, материалом, UV и тинтом.
	 */
	struct SpriteData
	{
		Guid textureGuid;
		Guid materialGuid;
		zF32 uv[4];
		zU32 tint;
	};

	/**
	 * @struct VisualPayload
	 * @brief Визуальная нагрузка объекта GameObject на базе std::variant.
	 */
	struct VisualPayload
	{
		eVisualType type;
		std::variant<std::monostate, SpriteData, SimpleMeshData, MultiMesh3DData> data;

		VisualPayload() noexcept
			: type(eVisualType::None)
			, data(std::monostate{})
		{
		}
	};
}


