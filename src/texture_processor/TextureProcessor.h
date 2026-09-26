#pragma once

#include "TextureProcessorTypes.h"
#include <expected>
#include <filesystem>
#include <span>
#include <vector>
#include <array>

namespace zzz::texture {

class TextureProcessor {
public:
    // -------------------------------------------------------------------------
    // 1. Получение характеристик изображения (быстрый probe без декодирования)
    // -------------------------------------------------------------------------
    static std::expected<ImageInfo, std::string> Probe(const std::filesystem::path& filePath);
    static std::expected<ImageInfo, std::string> Probe(std::span<const uint8_t> fileBytes);

    // -------------------------------------------------------------------------
    // 2. Конвертация, мипмапы и сжатие в целевой GPU-формат (BC7/BC5/BC4/RGBA8)
    // -------------------------------------------------------------------------
    static TextureConvertResult Convert(
        const std::filesystem::path& filePath,
        const TextureConvertOptions& options
    );

    static TextureConvertResult Convert(
        std::span<const uint8_t> fileBytes,
        const TextureConvertOptions& options
    );

    // -------------------------------------------------------------------------
    // 3. Сборка текстур в атлас с сохранением на диск (заглушка)
    // -------------------------------------------------------------------------
    static AtlasBuildResult BuildAtlas(
        const std::vector<std::filesystem::path>& inputFiles,
        const std::filesystem::path& outputImagePath,
        const AtlasOptions& options
    );

    // -------------------------------------------------------------------------
    // 4. Упаковка каналов / PBR ORM маски (заглушка)
    // -------------------------------------------------------------------------
    static TextureConvertResult PackChannels(
        const ChannelPackSources& sources,
        const TextureConvertOptions& options
    );

    // -------------------------------------------------------------------------
    // 5. Обработка карт нормалей (заглушка)
    // -------------------------------------------------------------------------
    static TextureConvertResult ProcessNormalMap(
        const std::filesystem::path& filePath,
        const NormalMapOptions& options
    );

    // -------------------------------------------------------------------------
    // 6. Сборка Cubemap из 6 граней (заглушка)
    // -------------------------------------------------------------------------
    static TextureConvertResult BuildCubemap(
        const std::array<std::filesystem::path, 6>& faceFiles,
        const TextureConvertOptions& options
    );
};

} // namespace zzz::texture
