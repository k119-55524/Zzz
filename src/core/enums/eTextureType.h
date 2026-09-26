#pragma once

#include <string_view>

#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eTextureType
	 * @brief Тип текстурного ресурса на GPU.
	 */
	enum class eTextureType : zU8
	{
		Texture2D      = 0, ///< Стандартная 2D текстура
		TextureCube    = 1, ///< Кубическая текстура (6 граней)
		Texture3D      = 2, ///< Объёмная 3D текстура
		Texture2DArray = 3  ///< Массив 2D текстур
	};

	[[nodiscard]] constexpr std::string_view ToString(eTextureType type)
	{
		switch (type)
		{
		case eTextureType::Texture2D:      return "Texture2D";
		case eTextureType::TextureCube:    return "TextureCube";
		case eTextureType::Texture3D:      return "Texture3D";
		case eTextureType::Texture2DArray: return "Texture2DArray";
		}
		THROW_RUNTIME("Необработанный eTextureType");
	}
}
