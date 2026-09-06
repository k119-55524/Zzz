#pragma once

#include <string_view>

namespace zzz::builder
{
	inline constexpr std::string_view c_ExtMeta        = ".meta";

	// Геометрия
	inline constexpr std::string_view c_ExtMeshObj     = ".obj";

	// Текстуры
	inline constexpr std::string_view c_ExtTexturePng  = ".png";

	// Материалы и шейдеры
	inline constexpr std::string_view c_ExtMaterial    = ".zmat";
	inline constexpr std::string_view c_ExtShaderHlsl  = ".hlsl";

	// Сцены, префабы и окна
	inline constexpr std::string_view c_ExtScene       = ".zs";
	inline constexpr std::string_view c_ExtPrefab      = ".zp";
	inline constexpr std::string_view c_ExtView        = ".zv";
}
