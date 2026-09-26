#include "TextureProcessor.h"

#include <fstream>
#include <format>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#ifdef _WIN32
#include <wrl/client.h>
#endif
#include <DirectXTex.h>

#include "core/logger/logger.h"
#include "core/constants/LogCategoryConstants.h"

Z_SET_LOG_CATEGORY(::zzz::core::Assets);

namespace zzz::texture {

namespace {

std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    const auto size = file.tellg();
    if (size <= 0) {
        return {};
    }
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

DXGI_FORMAT ToDxgiFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA8_SRGB:  return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case TextureFormat::BC1_UNORM:   return DXGI_FORMAT_BC1_UNORM;
        case TextureFormat::BC1_SRGB:    return DXGI_FORMAT_BC1_UNORM_SRGB;
        case TextureFormat::BC3_UNORM:   return DXGI_FORMAT_BC3_UNORM;
        case TextureFormat::BC3_SRGB:    return DXGI_FORMAT_BC3_UNORM_SRGB;
        case TextureFormat::BC4_UNORM:   return DXGI_FORMAT_BC4_UNORM;
        case TextureFormat::BC4_SNORM:   return DXGI_FORMAT_BC4_SNORM;
        case TextureFormat::BC5_UNORM:   return DXGI_FORMAT_BC5_UNORM;
        case TextureFormat::BC5_SNORM:   return DXGI_FORMAT_BC5_SNORM;
        case TextureFormat::BC7_UNORM:   return DXGI_FORMAT_BC7_UNORM;
        case TextureFormat::BC7_SRGB:    return DXGI_FORMAT_BC7_UNORM_SRGB;
        default:                         return DXGI_FORMAT_UNKNOWN;
    }
}

bool IsCompressedFormat(TextureFormat format) {
    return format != TextureFormat::RGBA8_UNORM && format != TextureFormat::RGBA8_SRGB;
}

} // namespace

// -----------------------------------------------------------------------------
// 1. Probe
// -----------------------------------------------------------------------------
std::expected<ImageInfo, std::string> TextureProcessor::Probe(const std::filesystem::path& filePath) {
    const auto bytes = ReadFileBytes(filePath);
    if (bytes.empty()) {
        return std::unexpected(std::format("Failed to read image file: '{}'", filePath.string()));
    }
    return Probe(std::span<const uint8_t>(bytes.data(), bytes.size()));
}

std::expected<ImageInfo, std::string> TextureProcessor::Probe(std::span<const uint8_t> fileBytes) {
    if (fileBytes.empty()) {
        return std::unexpected("Input file byte span is empty");
    }

    int w = 0;
    int h = 0;
    int comp = 0;
    const int result = stbi_info_from_memory(
        fileBytes.data(),
        static_cast<int>(fileBytes.size()),
        &w, &h, &comp
    );

    if (result == 0) {
        DOutWarning("TextureProcessor::Probe: stbi_info failed to parse image header");
        return std::unexpected("stbi_info failed to parse image header (unsupported or corrupted format)");
    }

    ImageInfo info;
    info.width = static_cast<uint32_t>(w);
    info.height = static_cast<uint32_t>(h);
    info.channels = static_cast<uint32_t>(comp);
    info.bitDepth = 8;
    info.hasAlpha = (comp == 2 || comp == 4);
    info.isDivisibleByTwo = (info.width > 0 && info.height > 0) &&
                           (info.width % 2 == 0) && (info.height % 2 == 0);
    info.isPowerOfTwo = (info.width > 0 && (info.width & (info.width - 1)) == 0) &&
                        (info.height > 0 && (info.height & (info.height - 1)) == 0);

    DOut("TextureProcessor::Probe: parsed image {}x{}, channels={}, hasAlpha={}, divisibleByTwo={}",
         info.width, info.height, info.channels, info.hasAlpha, info.isDivisibleByTwo);

    return info;
}

// -----------------------------------------------------------------------------
// 2. Convert
// -----------------------------------------------------------------------------
TextureConvertResult TextureProcessor::Convert(
    const std::filesystem::path& filePath,
    const TextureConvertOptions& options
) {
    const auto bytes = ReadFileBytes(filePath);
    if (bytes.empty()) {
        const std::string err = std::format("Failed to read image file: '{}'", filePath.string());
        DOutError("TextureProcessor::Convert: {}", err);
        return {
            .success = false,
            .errorMessage = err
        };
    }
    return Convert(std::span<const uint8_t>(bytes.data(), bytes.size()), options);
}

TextureConvertResult TextureProcessor::Convert(
    std::span<const uint8_t> fileBytes,
    const TextureConvertOptions& options
) {
    if (fileBytes.empty()) {
        DOutError("TextureProcessor::Convert: input file bytes buffer is empty");
        return {.success = false, .errorMessage = "Input file bytes buffer is empty"};
    }

    DOut("TextureProcessor::Convert: starting conversion, targetFormat={}, mips={}",
         static_cast<uint32_t>(options.targetFormat), options.generateMips);

    // 1. Декодирование пикселей (принудительно в 4 канала RGBA8)
    int w = 0;
    int h = 0;
    int comp = 0;
    unsigned char* rawPixels = stbi_load_from_memory(
        fileBytes.data(),
        static_cast<int>(fileBytes.size()),
        &w, &h, &comp, 4
    );

    if (!rawPixels) {
        const std::string reason = stbi_failure_reason() ? stbi_failure_reason() : "unknown";
        DOutError("TextureProcessor::Convert: failed to decode image: {}", reason);
        return {
            .success = false,
            .errorMessage = std::format("Failed to decode image data: {}", reason)
        };
    }

    if (w <= 0 || h <= 0) {
        stbi_image_free(rawPixels);
        DOutError("TextureProcessor::Convert: invalid dimensions ({}x{})", w, h);
        return {.success = false, .errorMessage = "Image has invalid dimensions (width or height <= 0)"};
    }

    // 2. Строгая валидация кратности 2
    if ((w % 2 != 0) || (h % 2 != 0)) {
        const std::string msg = std::format("Image dimensions ({}x{}) must be divisible by 2", w, h);
        stbi_image_free(rawPixels);
        DOutError("TextureProcessor::Convert: validation error: {}", msg);
        return {.success = false, .errorMessage = msg};
    }

    // 3. Формирование цепочки мипмапов в сыром RGBA8
    struct RawMip {
        uint32_t width{0};
        uint32_t height{0};
        std::vector<uint8_t> rgbaPixels;
    };

    std::vector<RawMip> rawMips;
    uint32_t currentW = static_cast<uint32_t>(w);
    uint32_t currentH = static_cast<uint32_t>(h);

    rawMips.push_back(RawMip{
        .width = currentW,
        .height = currentH,
        .rgbaPixels = std::vector<uint8_t>(rawPixels, rawPixels + (currentW * currentH * 4))
    });
    stbi_image_free(rawPixels);

    if (options.generateMips) {
        while (currentW > 1 || currentH > 1) {
            const uint32_t nextW = std::max(1u, currentW / 2);
            const uint32_t nextH = std::max(1u, currentH / 2);
            std::vector<uint8_t> nextPixels(nextW * nextH * 4);

            if (options.isSRGB) {
                stbir_resize_uint8_srgb(
                    rawMips.back().rgbaPixels.data(), static_cast<int>(currentW), static_cast<int>(currentH), static_cast<int>(currentW * 4),
                    nextPixels.data(), static_cast<int>(nextW), static_cast<int>(nextH), static_cast<int>(nextW * 4),
                    STBIR_RGBA
                );
            } else {
                stbir_resize_uint8_linear(
                    rawMips.back().rgbaPixels.data(), static_cast<int>(currentW), static_cast<int>(currentH), static_cast<int>(currentW * 4),
                    nextPixels.data(), static_cast<int>(nextW), static_cast<int>(nextH), static_cast<int>(nextW * 4),
                    STBIR_RGBA
                );
            }

            rawMips.push_back(RawMip{
                .width = nextW,
                .height = nextH,
                .rgbaPixels = std::move(nextPixels)
            });

            currentW = nextW;
            currentH = nextH;
        }
    }

    DOut("TextureProcessor::Convert: generated {} raw mip levels", rawMips.size());

    // 4. Сжатие или копирование каждого мип-уровня
    TextureConvertResult result;
    result.width = static_cast<uint32_t>(w);
    result.height = static_cast<uint32_t>(h);
    result.format = options.targetFormat;

    const bool compressed = IsCompressedFormat(options.targetFormat);
    const DXGI_FORMAT targetDxgi = ToDxgiFormat(options.targetFormat);

    for (const auto& mip : rawMips) {
        TextureMipDesc desc;
        desc.width = mip.width;
        desc.height = mip.height;
        desc.byteOffset = result.payload.size();

        if (!compressed) {
            // Несжатый RGBA8
            desc.rowPitch = mip.width * 4;
            desc.byteSize = mip.rgbaPixels.size();
            result.payload.insert(result.payload.end(), mip.rgbaPixels.begin(), mip.rgbaPixels.end());
        } else {
            // Компрессия через DirectXTex
            DirectX::Image srcImage;
            srcImage.width = mip.width;
            srcImage.height = mip.height;
            srcImage.format = options.isSRGB ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_R8G8B8A8_UNORM;
            srcImage.rowPitch = mip.width * 4;
            srcImage.slicePitch = mip.rgbaPixels.size();
            srcImage.pixels = const_cast<uint8_t*>(mip.rgbaPixels.data());

            DirectX::ScratchImage compressedImage;
            const HRESULT hr = DirectX::Compress(
                srcImage,
                targetDxgi,
                DirectX::TEX_COMPRESS_DEFAULT,
                DirectX::TEX_THRESHOLD_DEFAULT,
                compressedImage
            );

            if (FAILED(hr)) {
                DOutError("TextureProcessor::Convert: DirectXTex compression failed for mip {}x{}, HRESULT: 0x{:08X}",
                          mip.width, mip.height, static_cast<uint32_t>(hr));
                return {
                    .success = false,
                    .errorMessage = std::format("DirectXTex compression failed for mip {}x{}, HRESULT: 0x{:08X}",
                                                mip.width, mip.height, static_cast<uint32_t>(hr))
                };
            }

            const auto* image = compressedImage.GetImage(0, 0, 0);
            desc.rowPitch = static_cast<uint32_t>(image->rowPitch);
            desc.byteSize = compressedImage.GetPixelsSize();

            const auto* compressedBytes = compressedImage.GetPixels();
            result.payload.insert(result.payload.end(), compressedBytes, compressedBytes + desc.byteSize);
        }

        result.mips.push_back(desc);
    }

    result.success = true;
    DOut("TextureProcessor::Convert: finished successfully ({}x{}, {} mips, total payload {} bytes)",
         result.width, result.height, result.mips.size(), result.payload.size());
    return result;
}

// -----------------------------------------------------------------------------
// 3. BuildAtlas (Заглушка)
// -----------------------------------------------------------------------------
AtlasBuildResult TextureProcessor::BuildAtlas(
    const std::vector<std::filesystem::path>& /*inputFiles*/,
    const std::filesystem::path& /*outputImagePath*/,
    const AtlasOptions& /*options*/
) {
    return {
        .success = false,
        .errorMessage = "TextureProcessor::BuildAtlas is not implemented yet"
    };
}

// -----------------------------------------------------------------------------
// 4. PackChannels (Заглушка)
// -----------------------------------------------------------------------------
TextureConvertResult TextureProcessor::PackChannels(
    const ChannelPackSources& /*sources*/,
    const TextureConvertOptions& /*options*/
) {
    return {
        .success = false,
        .errorMessage = "TextureProcessor::PackChannels is not implemented yet"
    };
}

// -----------------------------------------------------------------------------
// 5. ProcessNormalMap (Заглушка)
// -----------------------------------------------------------------------------
TextureConvertResult TextureProcessor::ProcessNormalMap(
    const std::filesystem::path& /*filePath*/,
    const NormalMapOptions& /*options*/
) {
    return {
        .success = false,
        .errorMessage = "TextureProcessor::ProcessNormalMap is not implemented yet"
    };
}

// -----------------------------------------------------------------------------
// 6. BuildCubemap (Заглушка)
// -----------------------------------------------------------------------------
TextureConvertResult TextureProcessor::BuildCubemap(
    const std::array<std::filesystem::path, 6>& /*faceFiles*/,
    const TextureConvertOptions& /*options*/
) {
    return {
        .success = false,
        .errorMessage = "TextureProcessor::BuildCubemap is not implemented yet"
    };
}

} // namespace zzz::texture
