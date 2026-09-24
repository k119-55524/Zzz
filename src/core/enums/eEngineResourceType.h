#pragma once

#include <string_view>
#include "math/utils/Types.h"
#include "core/utils/ThrowWrappers.h"

namespace zzz::core
{
	/**
	 * @enum eEngineResourceType
	 * @brief Единый логический идентификатор типа контента и ресурса движка.
	 *
	 * @details Используется в реестре ассетов (AssetRegistry), мета-файлах (.meta),
	 *          системе импорта, инспекторе редактора и валидаторах GUID.
	 *          Отражает предметную сущность ресурса независимо от того, в каком архиве
	 *          он в конечном счёте упаковывается (package.dat, data.dat или остаётся исходником).
	 */
	enum class eEngineResourceType : zU16
	{
		Unknown = 0,

		// --- Ресурсы структуры проекта (package.dat) ---
		ProjectManifest, ///< Манифест проекта
		View,            ///< Общее представление / окно (.zview)
		PrimaryView,     ///< Основное окно (.zview)
		ChildView,       ///< Дочернее окно (.zview)
		IndependentView, ///< Независимое окно (.zview)
		Scene,           ///< Игровая сцена (.zscene)
		Prefab,          ///< Префаб (.zprefab)

		// --- Игровые бинарные ресурсы (data.dat) ---
		Mesh,            ///< 3D Сетка/Геометрия (.obj / кеш меша)
		Material,        ///< Материал (.zmaterial)
		Shader,          ///< Шейдер (.zshaders)
		Animation,       ///< Анимация
		Texture2D,       ///< 2D Текстура (DDS, PNG, JPEG)
		AudioClip,       ///< Аудиофайл (WAV, OGG, MP3)
		Video,           ///< Видео
		Font,            ///< Шрифт (TTF, OTF)
		BinaryData       ///< Произвольный бинарный буфер
	};

	[[nodiscard]] constexpr std::string_view ToString(eEngineResourceType type)
	{
		switch (type)
		{
		case eEngineResourceType::Unknown:         return "Unknown";
		case eEngineResourceType::ProjectManifest: return "ProjectManifest";
		case eEngineResourceType::View:            return "View";
		case eEngineResourceType::PrimaryView:     return "PrimaryView";
		case eEngineResourceType::ChildView:       return "ChildView";
		case eEngineResourceType::IndependentView: return "IndependentView";
		case eEngineResourceType::Scene:           return "Scene";
		case eEngineResourceType::Prefab:          return "Prefab";
		case eEngineResourceType::Mesh:            return "Mesh";
		case eEngineResourceType::Material:        return "Material";
		case eEngineResourceType::Shader:          return "Shader";
		case eEngineResourceType::Animation:       return "Animation";
		case eEngineResourceType::Texture2D:       return "Texture2D";
		case eEngineResourceType::AudioClip:       return "AudioClip";
		case eEngineResourceType::Video:           return "Video";
		case eEngineResourceType::Font:            return "Font";
		case eEngineResourceType::BinaryData:      return "BinaryData";
		}
		THROW_RUNTIME("Необработанный eEngineResourceType");
	}
}
