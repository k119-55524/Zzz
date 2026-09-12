#pragma once

#include <string_view>

#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eVisualType
	 * @brief Тип визуального представления объекта на сцене.
	 */
	enum class eVisualType : zU8
	{
		None = 0,
		Sprite,          // 2D спрайт
		Mesh2D,          // 2D меш
		SimpleMesh3D,    // 3D меш
		MultiMesh3D      // 3D составной меш
	};

	[[nodiscard]] constexpr std::string_view ToString(eVisualType type)
	{
		switch (type)
		{
		case eVisualType::None:         return "None";
		case eVisualType::Sprite:       return "Sprite";
		case eVisualType::Mesh2D:       return "Mesh2D";
		case eVisualType::SimpleMesh3D: return "SimpleMesh3D";
		case eVisualType::MultiMesh3D:  return "MultiMesh3D";
		}
		THROW_RUNTIME("Необработанный eVisualType");
	}
}
