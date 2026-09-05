#pragma once

#include <string_view>

namespace zzz::core::ext
{
	inline constexpr std::string_view Meta        = ".meta";

	// Геометрия
	inline constexpr std::string_view MeshObj     = ".obj";

	// Текстуры
	inline constexpr std::string_view TexturePng  = ".png";

	// Материалы и шейдеры
	inline constexpr std::string_view Material    = ".zmat";
	inline constexpr std::string_view ShaderHlsl  = ".hlsl";

	// Сцены, префабы и окна
	inline constexpr std::string_view Scene       = ".zs";
	inline constexpr std::string_view Prefab      = ".zp";
	inline constexpr std::string_view PrimaryView = ".zav";
	inline constexpr std::string_view ChildView   = ".zcv";
	inline constexpr std::string_view IndepView   = ".ziv";
}
