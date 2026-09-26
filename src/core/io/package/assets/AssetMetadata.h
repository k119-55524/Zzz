#pragma once

#include <array>
#include <cstddef>
#include <type_traits>

#include "math/utils/Types.h"
#include "core/enums/ePixelFormat.h"
#include "core/enums/eIndexFormat.h"
#include "core/enums/eTextureType.h"

namespace zzz::core
{
	/**
	 * @struct TextureMetadata
	 * @brief Метаданные текстурного ресурса в оглавлении (TOC).
	 * @details Хранится в оглавлении архива, позволяя создавать GPU-ресурс и настраивать
	 *          буфер загрузки без обращения к полезной нагрузке в .pak.
	 */
	struct TextureMetadata
	{
		zU32         width{ 0 };                             ///< Ширина в пикселях (кратна 2)
		zU32         height{ 0 };                            ///< Высота в пикселях (кратна 2)
		zU16         depth{ 1 };                             ///< Глубина (1 для 2D)
		zU16         arraySize{ 1 };                         ///< 1 для 2D, 6 для Cube
		ePixelFormat format{ ePixelFormat::Unknown };        ///< Формат пикселей / GPU сжатия
		zU8          mipCount{ 1 };                          ///< Количество мип-уровней (от 1 до 16)
		eTextureType textureType{ eTextureType::Texture2D }; ///< Тип ресурса (2D, Cube, 3D, 2DArray)
		zU8          flags{ 0 };                             ///< Флаги (бит 0: sRGB, бит 1: IsNormalMap)
		zU8          reserved[15]{ 0 };                      ///< Резерв под выравнивание и стриминг
	};

	/**
	 * @struct MeshMetadata
	 * @brief Метаданные 3D сетки/геометрии в оглавлении (TOC).
	 */
	struct MeshMetadata
	{
		zU32         vertexCount{ 0 };                   ///< Количество вершин
		zU32         vertexStride{ 0 };                  ///< Шаг вершины в байтах
		zU32         indexCount{ 0 };                    ///< Количество индексов
		zU16         vertexChannels{ 0 };                ///< Битовая маска атрибутов (Pos, Norm, UV0, UV1...)
		eIndexFormat indexFormat{ eIndexFormat::UInt16 };///< Формат индекса (UInt16 / UInt32)
		zU8          submeshCount{ 1 };                  ///< Количество подсеток/материалов
		zU8          reserved[16]{ 0 };                  ///< Резерв (LODs, границы BoundingBox)
	};

	/**
	 * @struct MaterialMetadata
	 * @brief Метаданные материала в оглавлении (TOC).
	 */
	struct MaterialMetadata
	{
		zU32 propertyCount{ 0 }; ///< Количество скалярных/векторных свойств
		zU32 textureCount{ 0 };  ///< Количество привязанных слотов текстур
		zU32 passMask{ 0 };      ///< Битовая маска поддерживаемых проходов рендеринга
		zU8  blendMode{ 0 };     ///< Режим смешивания (Opaque, Masked, Translucent)
		zU8  cullMode{ 0 };      ///< Режим отсечения граней (Back, Front, None)
		zU8  reserved[18]{ 0 };  ///< Резерв
	};

	/**
	 * @struct ShaderMetadata
	 * @brief Метаданные шейдерного байткода в оглавлении (TOC).
	 */
	struct ShaderMetadata
	{
		zU32 stageMask{ 0 };    ///< Битовая маска стадий (Vertex, Pixel, Compute)
		zU32 bytecodeSize{ 0 }; ///< Размер бинарного байткода в байтах
		zU32 gapiType{ 0 };     ///< Целевое графическое API (D3D12, Vulkan, Metal)
		zU8  reserved[20]{ 0 }; ///< Резерв
	};

	/**
	 * @struct AudioClipMetadata
	 * @brief Метаданные звукового ресурса в оглавлении (TOC).
	 */
	struct AudioClipMetadata
	{
		zU32 sampleRate{ 0 };    ///< Частота дискретизации в Гц (например, 44100, 48000)
		zU32 sampleCount{ 0 };   ///< Общее количество сэмплов
		zU16 channels{ 0 };      ///< Количество каналов (1 = моно, 2 = стерео)
		zU16 bitsPerSample{ 0 }; ///< Битность сэмпла (16, 24, 32)
		zU8  audioFormat{ 0 };   ///< Формат кодека (0 = PCM, 1 = Vorbis, 2 = MP3)
		zU8  reserved[19]{ 0 };  ///< Резерв
	};

	/**
	 * @struct AnimationMetadata
	 * @brief Метаданные скелетной/скейловой анимации в оглавлении (TOC).
	 */
	struct AnimationMetadata
	{
		zF32 duration{ 0.0f };   ///< Длительность анимации в секундах
		zF32 frameRate{ 30.0f }; ///< Частота кадров
		zU32 keyframeCount{ 0 }; ///< Количество ключевых кадров
		zU32 channelCount{ 0 };  ///< Количество анимируемых каналов / костей
		zU8  reserved[16]{ 0 };  ///< Резерв
	};

	/**
	 * @struct FontMetadata
	 * @brief Метаданные шрифтового атласа в оглавлении (TOC).
	 */
	struct FontMetadata
	{
		zU32 glyphCount{ 0 };   ///< Количество глифов
		zU16 atlasWidth{ 0 };   ///< Ширина атласа шрифта
		zU16 atlasHeight{ 0 };  ///< Высота атласа шрифта
		zU16 fontSize{ 0 };     ///< Базовый кегль шрифта в пикселях
		zU8  reserved[22]{ 0 }; ///< Резерв
	};

	/**
	 * @struct VideoMetadata
	 * @brief Метаданные видеопотока в оглавлении (TOC).
	 */
	struct VideoMetadata
	{
		zU32 width{ 0 };        ///< Ширина кадра
		zU32 height{ 0 };       ///< Высота кадра
		zF32 frameRate{ 0.0f }; ///< Частота кадров
		zF32 duration{ 0.0f };  ///< Длительность в секундах
		zU8  codec{ 0 };        ///< Идентификатор видеокодека
		zU8  reserved[15]{ 0 }; ///< Резерв
	};

	/**
	 * @struct BinaryDataMetadata
	 * @brief Метаданные произвольного бинарного ресурса в оглавлении (TOC).
	 */
	struct BinaryDataMetadata
	{
		zU32 userTag{ 0 };      ///< Пользовательский тег/подтип ресурса
		zU8  reserved[28]{ 0 }; ///< Резерв
	};

	/**
	 * @union AssetMetadata
	 * @brief Унифицированное объединение метаданных ресурса фиксированного размера (32 байта).
	 * @details Располагается непосредственно внутри записи оглавления (PackageEntry / DataPackageEntry).
	 *          Позволяет движку мгновенно узнать параметры любого ресурса при монтировании архива
	 *          без необходимости обращаться к телу внешних .pak файлов на диске.
	 */
	union AssetMetadata
	{
		TextureMetadata    texture;
		MeshMetadata       mesh;
		MaterialMetadata   material;
		ShaderMetadata     shader;
		AudioClipMetadata  audio;
		AnimationMetadata  animation;
		FontMetadata       font;
		VideoMetadata      video;
		BinaryDataMetadata binary;
		std::array<std::byte, 32> raw;

		constexpr AssetMetadata() noexcept
			: raw{}
		{
		}
	};

	static_assert(sizeof(TextureMetadata) == 32, "TextureMetadata must be exactly 32 bytes");
	static_assert(sizeof(MeshMetadata) == 32, "MeshMetadata must be exactly 32 bytes");
	static_assert(sizeof(MaterialMetadata) == 32, "MaterialMetadata must be exactly 32 bytes");
	static_assert(sizeof(ShaderMetadata) == 32, "ShaderMetadata must be exactly 32 bytes");
	static_assert(sizeof(AudioClipMetadata) == 32, "AudioClipMetadata must be exactly 32 bytes");
	static_assert(sizeof(AnimationMetadata) == 32, "AnimationMetadata must be exactly 32 bytes");
	static_assert(sizeof(FontMetadata) == 32, "FontMetadata must be exactly 32 bytes");
	static_assert(sizeof(VideoMetadata) == 32, "VideoMetadata must be exactly 32 bytes");
	static_assert(sizeof(BinaryDataMetadata) == 32, "BinaryDataMetadata must be exactly 32 bytes");
	static_assert(sizeof(AssetMetadata) == 32, "AssetMetadata must be exactly 32 bytes");
	static_assert(std::is_trivially_copyable_v<AssetMetadata>, "AssetMetadata must be trivially copyable");
}
