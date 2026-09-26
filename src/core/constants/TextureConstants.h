#pragma once

/**
 * @file TextureConstants.h
 * @brief Константы текстурной подсистемы, лимиты разрешений и блочной компрессии.
 */

#include "math/utils/Types.h"

namespace zzz::core
{
#pragma region Texture Constants
	/// Максимально допустимый размер стороны текстуры в пикселях (16384x16384)
	constexpr zU32 c_MaxTextureDimension = 16384;

	/// Максимально допустимый размер стороны текстурного атласа в пикселях (4096x4096)
	constexpr zU32 c_MaxAtlasDimension = 4096;

	/// Минимальный размер стороны текстуры в пикселях для блочных компрессоров (BC / ASTC)
	constexpr zU32 c_MinTextureDimension = 2;
#pragma endregion
}
