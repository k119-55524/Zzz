#pragma once

#include "core/types/BaseTypes.h"

namespace zzz::core
{
	/**
	 * @enum eVisualType
	 * @brief Тип визуального представления объекта на сцене.
	 */
	enum class eVisualType : zU8
	{
		None = 0,

		// 2D:
		Sprite,          // 2D спрайт
		Mesh2D,          // 2D полигональный меш

		// 3D:
		SimpleMesh3D,    // 3D простой монолитный меш (1 сетка + 1 материал)
		MultiMesh3D,     // 3D составной меш (N сабмешей + N материалов по индексу)

		// Общие / Системные:
		ParticleSystem   // Система частиц
	};
}
