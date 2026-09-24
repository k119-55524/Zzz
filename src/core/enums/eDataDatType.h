#pragma once

#include <string_view>

#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eDataDatType
	 * @brief Физические типы записей в оглавлении архива data.dat (контейнер сырых игровых ресурсов).
	 *
	 * @details Определяет тип ресурса полезной нагрузки (меши, материалы, шейдеры, текстуры и т.д.),
	 *          упакованного в бинарный архив c_DataPackageRelativePath.
	 *          В data.dat никогда не попадают сцены, манифест или представления.
	 */
	enum class eDataDatType : zU32
	{
		Mesh       = 1, ///< 3D Сетка/Геометрия (Vertex/Index buffers)
		Material   = 2, ///< Материал (.zmaterial)
		Shader     = 3, ///< Шейдерная программа / байткод (.zshaders)
		Animation  = 4, ///< Анимационные данные
		Texture2D  = 5, ///< 2D Текстура (DDS, PNG, RGBA)
		AudioClip  = 6, ///< Звуковой файл (WAV, OGG, MP3)
		Video      = 7, ///< Видеопоток
		Font       = 8, ///< Шрифт (TTF, OTF, атласы)
		BinaryData = 9  ///< Произвольный бинарный буфер данных
	};

	[[nodiscard]] constexpr std::string_view ToString(eDataDatType type)
	{
		switch (type)
		{
		case eDataDatType::Mesh:       return "Mesh";
		case eDataDatType::Material:   return "Material";
		case eDataDatType::Shader:     return "Shader";
		case eDataDatType::Animation:  return "Animation";
		case eDataDatType::Texture2D:  return "Texture2D";
		case eDataDatType::AudioClip:  return "AudioClip";
		case eDataDatType::Video:      return "Video";
		case eDataDatType::Font:       return "Font";
		case eDataDatType::BinaryData: return "BinaryData";
		}
		THROW_RUNTIME("Необработанный eDataDatType");
	}
}
