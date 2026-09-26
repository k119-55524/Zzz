#pragma once

#include <string_view>

namespace zzz::builder
{
	inline constexpr std::string_view c_ExtMeta        = ".meta";

	// Геометрия
	inline constexpr std::string_view c_ExtMeshObj     = ".obj";

	// Текстуры
	inline constexpr std::string_view c_ExtTexturePng  = ".png";
	inline constexpr std::string_view c_ExtTextureJpg  = ".jpg";
	inline constexpr std::string_view c_ExtTextureJpeg = ".jpeg";
	inline constexpr std::string_view c_ExtTextureTga  = ".tga";
	inline constexpr std::string_view c_ExtTextureBmp  = ".bmp";

	// Аудио
	inline constexpr std::string_view c_ExtAudioWav    = ".wav";
	inline constexpr std::string_view c_ExtAudioOgg    = ".ogg";

	// Материалы и шейдеры
	inline constexpr std::string_view c_ExtMaterial    = ".zmaterial";
	inline constexpr std::string_view c_ExtShaderHlsl  = ".zshaders";

	// Сцены, префабы и окна
	inline constexpr std::string_view c_ExtScene       = ".zscene";
	inline constexpr std::string_view c_ExtPrefab      = ".zprefab";
	inline constexpr std::string_view c_ExtView        = ".zview";
}
