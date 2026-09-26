#pragma once

/**
 * @file TextureBuilder.h
 * @brief Подсистема инспекции, конвертации, генерации мипмапов и сжатия текстур.
 *
 * @details Класс TextureBuilder предоставляет функционал для подготовки текстурных данных:
 *          - Быстрое извлечение метаданных (Probe) без декодирования всего изображения.
 *          - Конвертация форматов, даунскейлинг и построение мип-цепочек (stb_image, stb_image_resize2).
 *          - Блочная компрессия в нативные GPU-форматы (BC1-BC7) с помощью DirectXTex.
 *          - Упаковка PBR-каналов (ORM), обработка карт нормалей, сборка атласов и cubemap.
 */
#include <span>
#include <array>
#include <vector>
#include <expected>
#include <filesystem>

#include "math/utils/Types.h"
#include "TextureBuilderTypes.h"

namespace zzz::texture
{
	/**
	 * @class TextureBuilder
	 * @brief Билдер обработки, запекания, построения мипмапов и компрессии текстур.
	 *
	 * @details Класс инкапсулирует настройки процесса импорта (максимальные разрешения,
	 *          валидация размерностей) и предоставляет полный пайплайн подготовки текстур
	 *          для рендеринга и упаковки в игровые архивы:
	 *
	 *          - **Инспекция метаданных (Probe):** Чтение информации о размерах, каналах
	 *            и битовой глубине без выделения памяти под пиксели (через `stbi_info`).
	 *          - **Декодирование:** Поддержка форматов художников (PNG, JPG, TGA, BMP, HDR)
	 *            с распаковкой в 32-битный RGBA8.
	 *          - **Валидация:** Проверка кратности сторон 2 (необходимо для блочных компрессоров)
	 *            и соблюдения максимального допустимого разрешения.
	 *          - **Генерация мипмапов:** Построение цепочки мип-уровней до 1x1 с качественной
	 *            гамма-коррекцией (`stbir_resize_uint8_srgb` / `stbir_resize_uint8_linear`).
	 *          - **Аппаратная компрессия:** Сжатие в GPU-форматы BC1–BC7 с помощью библиотеки
	 *            DirectXTex (BC7 для Albedo, BC5 для нормалей, BC4 для масок).
	 *          - **Составные текстуры:** Упаковка каналов (ORM: Occlusion, Roughness, Metallic),
	 *            предобработка карт нормалей (инверсия Y, нормализация), сборка атласов и Cubemap.
	 *
	 * @par Многопоточность (Thread Safety):
	 *      Методы обработки (`Probe`, `Convert`, и др.) являются константными (`const`)
	 *      и потокобезопасными относительно состояния билдера. В многопоточных пайплайнах
	 *      сборки ресурсов рекомендуется создавать по одному экземпляру `TextureBuilder`
	 *      на каждый рабочий поток (worker thread) для независимого управления конфигурацией.
	 * @par Пример использования:
	 * @code{.cpp}
	 * using namespace zzz::texture;
	 *
	 * // 1. Инициализация билдера
	 * TextureBuilder builder;
	 *
	 * // 2. Быстрая проверка метаданных
	 * auto probeInfo = builder.Probe(fileBytes);
	 * if (probeInfo && probeInfo->IsPowerOfTwo()) {
	 *     // 3. Конвертация в BC7 sRGB с генерацией мипмапов
	 *     TextureConvertOptions options{
	 *         .targetFormat = core::ePixelFormat::BC7_SRGB,
	 *         .generateMips = true
	 *     };
	 *     auto result = builder.Convert(fileBytes, options);
	 *     if (result) {
	 *         // Использование result->payload и result->metadata
	 *     }
	 * }
	 * @endcode
	 *
	 * @see TextureConvertOptions
	 * @see TextureConvertResult
	 * @see ImageInfo
	 */
	class TextureBuilder
	{
	public:
		TextureBuilder() = default;

		/**
		 * @brief Быстро считывает характеристики изображения из буфера байтов в памяти.
		 * @details Анализирует заголовок через stbi_info_from_memory без декодирования пиксельного массива.
		 * @param fileBytes Срез байтов исходного файла изображения.
		 * @return ImageInfo со свойствами текстуры или строка с описанием ошибки.
		 */
		[[nodiscard]] std::expected<ImageInfo, std::string> Probe(std::span<const zU8> fileBytes) const;

		/**
		 * @brief Выполняет декодирование из памяти, валидацию, генерацию мипмапов и сжатие в целевой формат.
		 * @param fileBytes Срез байтов исходного файла изображения.
		 * @param options Параметры конвертации (целевой формат, генерация мипов).
		 * @return TextureConvertResult с метаданными ресурса и упакованными байтами payload.
		 */
		[[nodiscard]] std::expected<TextureConvertResult, std::string> Convert(std::span<const zU8> fileBytes, const TextureConvertOptions& options) const;

		/**
		 * @brief Упаковывает набор отдельных спрайтов/текстур в единый атлас.
		 * @param inputFiles Список путей к исходным изображениям.
		 * @param outputImagePath Путь для сохранения результирующего файла атласа.
		 * @param options Параметры атласа (максимальное разрешение, отступы, кратность степени двойки).
		 * @return AtlasBuildResult с координатами спрайтов на атласе (UV) или текстом ошибки.
		 */
		[[nodiscard]] std::expected<AtlasBuildResult, std::string> BuildAtlas(const std::vector<std::filesystem::path>& inputFiles, const std::filesystem::path& outputImagePath, const AtlasOptions& options) const;

		/**
		 * @brief Упаковывает отдельные текстуры в цветовые каналы единого PBR-композита (например, ORM: R=AO, G=Roughness, B=Metallic).
		 * @param sources Пути к исходным одноканальным или цветным текстурам для каждого канала.
		 * @param options Параметры конвертации и сжатия результирующего композита.
		 * @return TextureConvertResult с упакованной и сжатой составной текстурой или текстом ошибки.
		 */
		[[nodiscard]] std::expected<TextureConvertResult, std::string> PackChannels(const ChannelPackSources& sources, const TextureConvertOptions& options) const;

		/**
		 * @brief Выполняет специализированную обработку карт нормалей.
		 * @details Поддерживает инверсию оси Y (зелёного канала) для согласования с DirectX/OpenGL,
		 *          ренормализацию векторов и сжатие в двухканальный формат BC5.
		 * @param filePath Путь к файлу карты нормалей.
		 * @param options Параметры обработки (инверсия Y, нормализация, целевой формат).
		 * @return TextureConvertResult с обработанной картой нормалей или текстом ошибки.
		 */
		[[nodiscard]] std::expected<TextureConvertResult, std::string> ProcessNormalMap(const std::filesystem::path& filePath, const NormalMapOptions& options) const;

		/**
		 * @brief Собирает кубическую карту (Cubemap) из 6 отдельных текстур граней.
		 * @param faceFiles Массив путей к 6 граням (+X, -X, +Y, -Y, +Z, -Z).
		 * @param options Параметры конвертации и генерации мип-уровней для граней.
		 * @return TextureConvertResult с итоговой кубической текстурой или текстом ошибки.
		 */
		[[nodiscard]] std::expected<TextureConvertResult, std::string> BuildCubemap(const std::array<std::filesystem::path, 6>& faceFiles, const TextureConvertOptions& options) const;
	};
} // namespace zzz::texture
