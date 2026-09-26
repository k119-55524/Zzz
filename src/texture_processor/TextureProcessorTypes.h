#pragma once

#include "TextureFormat.h"

#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>
#include <span>
#include <array>

namespace zzz::texture {

/// 1. Характеристики изображения с диска/памяти
struct ImageInfo {
    uint32_t width{0};
    uint32_t height{0};
    uint32_t channels{0};       // 1 (R), 2 (RG), 3 (RGB), 4 (RGBA)
    uint32_t bitDepth{8};       // 8, 16, 32
    bool hasAlpha{false};
    bool isDivisibleByTwo{false};
    bool isPowerOfTwo{false};
};

/// 2. Описание мип-уровня и параметры/результат конвертации
struct TextureMipDesc {
    uint32_t width{0};
    uint32_t height{0};
    uint32_t rowPitch{0};
    size_t byteOffset{0};
    size_t byteSize{0};
};

struct TextureConvertOptions {
    TextureFormat targetFormat{TextureFormat::BC7_SRGB};
    bool generateMips{true};
    bool isSRGB{true};
};

struct TextureConvertResult {
    bool success{false};
    std::string errorMessage;
    uint32_t width{0};
    uint32_t height{0};
    TextureFormat format{TextureFormat::Unknown};
    std::vector<TextureMipDesc> mips;
    std::vector<uint8_t> payload; // Непрерывный массив байтов всех мипов
};

/// 3. Параметры и результат сборки атласа
struct AtlasOptions {
    uint32_t maxAtlasWidth{4096};
    uint32_t maxAtlasHeight{4096};
    uint32_t paddingPixels{2};
    bool powerOfTwo{true};
};

struct AtlasSpriteRect {
    std::string spriteId;
    uint32_t x{0};
    uint32_t y{0};
    uint32_t width{0};
    uint32_t height{0};
    float uMin{0.0f};
    float vMin{0.0f};
    float uMax{1.0f};
    float vMax{1.0f};
};

struct AtlasBuildResult {
    bool success{false};
    std::string errorMessage;
    uint32_t atlasWidth{0};
    uint32_t atlasHeight{0};
    std::vector<AtlasSpriteRect> sprites;
};

/// 4. Источники для упаковки каналов (ORM)
struct ChannelPackSources {
    std::filesystem::path redChannelPath;   // например AO
    std::filesystem::path greenChannelPath; // например Roughness
    std::filesystem::path blueChannelPath;  // например Metallic
    std::filesystem::path alphaChannelPath; // опционально
};

/// 5. Параметры обработки карт нормалей
struct NormalMapOptions {
    bool flipY{false};        // Инвертировать зелёный канал
    bool normalize{true};     // Нормализовать векторы до единичной длины
    TextureFormat targetFormat{TextureFormat::BC5_UNORM};
};

} // namespace zzz::texture
