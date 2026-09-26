#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "math/utils/Types.h"
#include "core/enums/ePixelFormat.h"

/**
 * @file TextureBuilderTypes.h
 * @brief Базовые типы данных, структуры настроек и результаты обработки текстур.
 */
 namespace zzz::texture
{
	/**
	 * @struct ImageInfo
	 * @brief Характеристики изображения, полученные в результате инспекции (Probe).
	 */
	struct ImageInfo
	{
		/// Ширина изображения в пикселях.
		zU32 width{ 0 };

		/// Высота изображения в пикселях.
		zU32 height{ 0 };

		/// Количество каналов: 1 (R/Grayscale), 2 (RG), 3 (RGB), 4 (RGBA).
		zU32 channels{ 0 };

		/// Глубина цвета одного канала в битах (8, 16, 32).
		zU32 bitDepth{ 8 };

		/// Признак наличия альфа-канала (channels == 2 или 4).
		bool hasAlpha{ false };

		/// Проверка кратности сторон двум (width % 2 == 0 && height % 2 == 0).
		[[nodiscard]] constexpr bool IsDivisibleByTwo() const noexcept
		{
			return (width > 0 && height > 0) && (width % 2 == 0) && (height % 2 == 0);
		}

		/// Проверка кратности сторон степени двойки (Power of Two).
		[[nodiscard]] constexpr bool IsPowerOfTwo() const noexcept
		{
			return (width > 0 && (width & (width - 1)) == 0) && (height > 0 && (height & (height - 1)) == 0);
		}
	};

	/**
	 * @struct TextureMipDesc
	 * @brief Описание отдельного мип-уровня в итоговом буфере полезной нагрузки.
	 */
	struct TextureMipDesc
	{
		/// Ширина данного мип-уровня в пикселях.
		zU32 width{ 0 };

		/// Высота данного мип-уровня в пикселях.
		zU32 height{ 0 };

		/// Шаг строки в байтах (Row Pitch / Stride).
		zU32 rowPitch{ 0 };

		/// Смещение данных мип-уровня от начала буфера payload в байтах.
		size_t byteOffset{ 0 };

		/// Размер данных мип-уровня в байтах.
		size_t byteSize{ 0 };
	};

	/**
	 * @struct TextureConvertOptions
	 * @brief Параметры конвертации, генерации мипмапов и компрессии текстуры.
	 */
	struct TextureConvertOptions
	{
		/// Целевой GPU-формат сжатия (BC7, BC5, BC4, BC1, RGBA8 и т.д.). Гамма (sRGB/Linear) определяется форматом.
		core::ePixelFormat targetFormat{ core::ePixelFormat::BC7_SRGB };

		/// Флаг генерации цепочки мип-уровней до 1x1 пикселя.
		bool generateMips{ true };
	};

	/**
	 * @struct TextureConvertResult
	 * @brief Результат конвертации и сжатия текстуры.
	 */
	struct TextureConvertResult
	{
		/// Успешность операции конвертации.
		bool success{ false };

		/// Описание ошибки в случае неудачи.
		std::string errorMessage;

		/// Итоговая ширина текстуры (мип 0).
		zU32 width{ 0 };

		/// Итоговая высота текстуры (мип 0).
		zU32 height{ 0 };

		/// Итоговый формат текстуры.
		core::ePixelFormat format{ core::ePixelFormat::Unknown };

		/// Список дескрипторов всех сгенерированных мип-уровней.
		std::vector<TextureMipDesc> mips;

		/// Непрерывный массив байтов всех сжатых/несжатых мип-уровней.
		std::vector<zU8> payload;
	};

	/**
	 * @struct AtlasOptions
	 * @brief Параметры упаковки текстурного атласа.
	 */
	struct AtlasOptions
	{
		/// Максимальная допустимая ширина атласа в пикселях.
		zU32 maxAtlasWidth{ 4096 };

		/// Максимальная допустимая высота атласа в пикселях.
		zU32 maxAtlasHeight{ 4096 };

		/// Отступ между спрайтами в пикселях для предотвращения артефактов фильтрации (bleeding).
		zU32 paddingPixels{ 2 };

		/// Требование кратности размеров атласа степени двойки (PoT).
		bool powerOfTwo{ true };
	};

	/**
	 * @struct AtlasSpriteRect
	 * @brief Прямоугольная область спрайта внутри текстурного атласа.
	 */
	struct AtlasSpriteRect
	{
		/// Идентификатор спрайта (имя исходного файла).
		std::string spriteId;

		/// Координата X левого верхнего угла спрайта на атласе в пикселях.
		zU32 x{ 0 };

		/// Координата Y левого верхнего угла спрайта на атласе в пикселях.
		zU32 y{ 0 };

		/// Ширина спрайта в пикселях.
		zU32 width{ 0 };

		/// Высота спрайта в пикселях.
		zU32 height{ 0 };

		/// Нормализованная координата U минимальной границы [0..1].
		zF32 uMin{ 0.0f };

		/// Нормализованная координата V минимальной границы [0..1].
		zF32 vMin{ 0.0f };

		/// Нормализованная координата U максимальной границы [0..1].
		zF32 uMax{ 1.0f };

		/// Нормализованная координата V максимальной границы [0..1].
		zF32 vMax{ 1.0f };
	};

	/**
	 * @struct AtlasBuildResult
	 * @brief Результат упаковки текстурного атласа.
	 */
	struct AtlasBuildResult
	{
		/// Успешность операции упаковки атласа.
		bool success{ false };

		/// Описание ошибки в случае неудачи.
		std::string errorMessage;

		/// Итоговая ширина атласа в пикселях.
		zU32 atlasWidth{ 0 };

		/// Итоговая высота атласа в пикселях.
		zU32 atlasHeight{ 0 };

		/// Список упакованных спрайтов с их координатами на атласе.
		std::vector<AtlasSpriteRect> sprites;
	};

	/**
	 * @struct ChannelPackSources
	 * @brief Пути к исходным изображениям для упаковки в PBR ORM маску.
	 */
	struct ChannelPackSources
	{
		/// Исходный файл для красного канала (например, Ambient Occlusion).
		std::filesystem::path redChannelPath;

		/// Исходный файл для зелёного канала (например, Roughness).
		std::filesystem::path greenChannelPath;

		/// Исходный файл для синего канала (например, Metallic).
		std::filesystem::path blueChannelPath;

		/// Опциональный файл для альфа-канала (например, Height / Displacement / Opacity).
		std::filesystem::path alphaChannelPath;
	};

	/**
	 * @struct NormalMapOptions
	 * @brief Параметры предобработки карт нормалей.
	 */
	struct NormalMapOptions
	{
		/// Инвертировать ось Y (зелёный канал) для DirectX / OpenGL согласования.
		bool flipY{ false };

		/// Принудительно нормализовать векторы до единичной длины перед сжатием.
		bool normalize{ true };

		/// Целевой GPU-формат для карты нормалей (по умолчанию BC5_UNORM).
		core::ePixelFormat targetFormat{ core::ePixelFormat::BC5_UNORM };
	};

} // namespace zzz::texture
