#pragma once

#include <vector>
#include "core/types/BaseTypes.h"
#include "core/enums/eVisualType.h"
#include "core/utils/Guid.h"

using namespace zzz::core;

namespace zzz::engine
{
	/**
	 * @struct VisualPayload
	 * @brief Визуальная нагрузка объекта GameObject на сцене.
	 */
	struct VisualPayload
	{
		eVisualType       type{ eVisualType::None };
		Guid              resourceGuid;                    ///< MeshGuid, TextureGuid или EffectGuid
		Guid              materialGuid;                    ///< Основной материал (для SimpleMesh3D, Sprite, Mesh2D)
		zF32              uv[4]{ 0.0f, 0.0f, 1.0f, 1.0f }; ///< u0, v0, u1, v1 для Sprite
		zU32              tint{ 0xFFFFFFFF };              ///< Цвет RGBA8 для Sprite

		// Для MultiMesh3D:
		std::vector<Guid> submeshes;                       ///< submeshes[i] — сетка
		std::vector<Guid> materials;                       ///< materials[i] — материал для submeshes[i]
	};
}
