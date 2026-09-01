#pragma once

#include "math/utils/Types.h"

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
		Texture2D,      ///< 2D Текстура (DDS, PNG, JPEG, RGBA)
		Mesh,           ///< 3D Сетка/Геометрия (Vertex/Index buffers, Submeshes)
		Material,       ///< Материал (.zmat)
		Shader,         ///< Шейдер (скомпилированный байткод DXIL / SPIR-V / MetalLib)
		AudioClip,      ///< Аудиофайл (WAV, OGG, MP3)
		Font,           ///< Шрифт (TTF, OTF, SDF/MSDF текстурные атласы)
		Scene,          ///< Сцена (дерево GameObject, компоненты)
		Prefab,         ///< Префаб объекта
		BinaryData      ///< Произвольный бинарный буфер
	};
}
