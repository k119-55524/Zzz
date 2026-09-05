#pragma once

#include <string_view>
#include "core/utils/Types.h"
#include "core/utils/ThrowWrappers.h"
#include "core/enums/eResourceType.h"
#include "core/io/AssetFileExtensions.h"

namespace zzz::core
{
	/**
	 * @enum eResourceStorageKind
	 * @brief Способ размещения и хранения ресурса в структуре билда.
	 */
	enum class eResourceStorageKind : zU8
	{
		PackageArchive,   ///< assets/package.dat (манифест, сцены, окна)
		DataArchive,      ///< assets/data/data.dat (меши, материалы, шейдеры, префабы)
		DedicatedFolder   ///< assets/data/<subdir>/ (текстуры, аудио, видео, шрифты)
	};

	constexpr std::string_view ToString(eResourceStorageKind kind)
	{
		switch (kind)
		{
		case eResourceStorageKind::PackageArchive: return "PackageArchive";
		case eResourceStorageKind::DataArchive:    return "DataArchive";
		case eResourceStorageKind::DedicatedFolder: return "DedicatedFolder";
		}
		THROW_RUNTIME("Необработанный eResourceStorageKind");
	}

	/**
	 * @struct ResourceTypeTraits
	 * @brief Свойства хранения типа ресурса.
	 */
	struct ResourceTypeTraits
	{
		eResourceStorageKind storageKind{ eResourceStorageKind::DataArchive };
		std::string_view relativeDir{};
	};

	constexpr ResourceTypeTraits GetResourceStorageTraits(eResourceType type)
	{
		switch (type)
		{
		case eResourceType::ProjectManifest:
		case eResourceType::PrimaryView:
		case eResourceType::ChildView:
		case eResourceType::IndependentView:
		case eResourceType::Scene:
			return { eResourceStorageKind::PackageArchive, "" };

		case eResourceType::Mesh:
		case eResourceType::Prefab:
		case eResourceType::Material:
		case eResourceType::Shader:
		case eResourceType::Animation:
			return { eResourceStorageKind::DataArchive, "" };

		case eResourceType::Texture2D:
			return { eResourceStorageKind::DedicatedFolder, "textures" };

		case eResourceType::AudioClip:
			return { eResourceStorageKind::DedicatedFolder, "audio" };

		case eResourceType::Video:
			return { eResourceStorageKind::DedicatedFolder, "video" };

		case eResourceType::Font:
			return { eResourceStorageKind::DedicatedFolder, "fonts" };

		case eResourceType::BinaryData:
		case eResourceType::Unknown:
		default:
			return { eResourceStorageKind::DataArchive, "" };
		}
	}
}
