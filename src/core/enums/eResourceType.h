#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eResourceType
	 * @brief Идентификатор гранулярного типа контента/ресурса движка (1 байт).
	 *
	 * @details Используется в мета-файлах (.meta), реестре ассетов, системе импорта и VFS.
	 *
	 * @note Отличие от ePackage (src/core/enums/ePackage.h):
	 *       - ePackage (zU32) определяет тип контейнера/архива верхнего уровня (Scene, Prefab, ProjectManifest, BinaryAsset).
	 *       - eResourceType (zU8) определяет тип конкретного ассета внутри пакета (Texture2D, Mesh, Material, Shader, Font...).
	 */
	enum class eResourceType : zU8
	{
		Unknown = 0,
		ProjectManifest, ///< Манифест проекта
		View,            ///< Окно / представление (.zview)
		Scene,           ///< Сцена (.zscene)
		Prefab,          ///< Префаб (.zprefab)
		Mesh,            ///< 3D Сетка/Геометрия (Vertex/Index buffers)
		Material,        ///< Материал (.zmaterial)
		Shader,          ///< Шейдер (.zshaders / байткод)
		Animation,       ///< Анимация
		Texture2D,       ///< 2D Текстура (DDS, PNG, JPEG, RGBA)
		AudioClip,       ///< Аудиофайл (WAV, OGG, MP3)
		Video,           ///< Видео
		Font,            ///< Шрифт (TTF, OTF, атласы)
		BinaryData       ///< Произвольный бинарный буфер
	};

	constexpr std::string_view ToString(eResourceType type)
	{
		switch (type)
		{
		case eResourceType::Unknown:         return "Unknown";
		case eResourceType::ProjectManifest: return "ProjectManifest";
		case eResourceType::View:            return "View";
		case eResourceType::Scene:           return "Scene";
		case eResourceType::Prefab:          return "Prefab";
		case eResourceType::Mesh:            return "Mesh";
		case eResourceType::Material:        return "Material";
		case eResourceType::Shader:          return "Shader";
		case eResourceType::Animation:       return "Animation";
		case eResourceType::Texture2D:       return "Texture2D";
		case eResourceType::AudioClip:       return "AudioClip";
		case eResourceType::Video:           return "Video";
		case eResourceType::Font:            return "Font";
		case eResourceType::BinaryData:      return "BinaryData";
		}
		THROW_RUNTIME("Необработанный eResourceType");
	}
}
