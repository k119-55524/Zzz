#include <gtest/gtest.h>
#include "texture_processor/TextureProcessor.h"

using namespace zzz::texture;

namespace {

std::vector<uint8_t> CreateTestTga(uint16_t w, uint16_t h, uint8_t bpp = 32) {
    std::vector<uint8_t> data(18 + static_cast<size_t>(w) * h * (bpp / 8), 0);
    data[2] = 2; // Uncompressed true-color image
    data[12] = static_cast<uint8_t>(w & 0xFF);
    data[13] = static_cast<uint8_t>((w >> 8) & 0xFF);
    data[14] = static_cast<uint8_t>(h & 0xFF);
    data[15] = static_cast<uint8_t>((h >> 8) & 0xFF);
    data[16] = bpp; // 32 bits per pixel
    data[17] = 0x28; // Origin top-left, 8-bit alpha

    for (size_t i = 18; i < data.size(); i += 4) {
        data[i + 0] = 255; // B
        data[i + 1] = 128; // G
        data[i + 2] = 64;  // R
        data[i + 3] = 255; // A
    }
    return data;
}

} // namespace

TEST(TextureProcessorTests, ProbeValidTga) {
    const auto tgaData = CreateTestTga(16, 16);
    const auto probeResult = TextureProcessor::Probe(tgaData);

    ASSERT_TRUE(probeResult.has_value());
    const auto& info = probeResult.value();
    EXPECT_EQ(info.width, 16u);
    EXPECT_EQ(info.height, 16u);
    EXPECT_EQ(info.channels, 4u);
    EXPECT_TRUE(info.hasAlpha);
    EXPECT_TRUE(info.isDivisibleByTwo);
    EXPECT_TRUE(info.isPowerOfTwo);
}

TEST(TextureProcessorTests, ProbeNonPowerOfTwoDivisibleByTwo) {
    const auto tgaData = CreateTestTga(6, 10);
    const auto probeResult = TextureProcessor::Probe(tgaData);

    ASSERT_TRUE(probeResult.has_value());
    const auto& info = probeResult.value();
    EXPECT_EQ(info.width, 6u);
    EXPECT_EQ(info.height, 10u);
    EXPECT_TRUE(info.isDivisibleByTwo);
    EXPECT_FALSE(info.isPowerOfTwo);
}

TEST(TextureProcessorTests, ProbeEmptyBufferFails) {
    const std::vector<uint8_t> empty;
    const auto probeResult = TextureProcessor::Probe(empty);
    EXPECT_FALSE(probeResult.has_value());
}

TEST(TextureProcessorTests, ConvertOddDimensionsFailsValidation) {
    const auto oddTga = CreateTestTga(5, 5);
    TextureConvertOptions options{
        .targetFormat = TextureFormat::RGBA8_UNORM,
        .generateMips = false,
        .isSRGB = false
    };

    const auto result = TextureProcessor::Convert(oddTga, options);
    EXPECT_FALSE(result.success);
    EXPECT_NE(result.errorMessage.find("must be divisible by 2"), std::string::npos);
}

TEST(TextureProcessorTests, ConvertRgba8WithMips) {
    const auto tgaData = CreateTestTga(4, 4);
    TextureConvertOptions options{
        .targetFormat = TextureFormat::RGBA8_UNORM,
        .generateMips = true,
        .isSRGB = false
    };

    const auto result = TextureProcessor::Convert(tgaData, options);
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.width, 4u);
    EXPECT_EQ(result.height, 4u);
    EXPECT_EQ(result.format, TextureFormat::RGBA8_UNORM);

    // 4x4 -> 2x2 -> 1x1 = 3 levels
    ASSERT_EQ(result.mips.size(), 3u);
    EXPECT_EQ(result.mips[0].width, 4u);
    EXPECT_EQ(result.mips[0].height, 4u);
    EXPECT_EQ(result.mips[0].byteSize, 4u * 4u * 4u); // 64 bytes

    EXPECT_EQ(result.mips[1].width, 2u);
    EXPECT_EQ(result.mips[1].height, 2u);
    EXPECT_EQ(result.mips[1].byteSize, 2u * 2u * 4u); // 16 bytes

    EXPECT_EQ(result.mips[2].width, 1u);
    EXPECT_EQ(result.mips[2].height, 1u);
    EXPECT_EQ(result.mips[2].byteSize, 1u * 1u * 4u); // 4 bytes

    EXPECT_EQ(result.payload.size(), 64u + 16u + 4u);
}

TEST(TextureProcessorTests, ConvertBc7Compression) {
    const auto tgaData = CreateTestTga(8, 8);
    TextureConvertOptions options{
        .targetFormat = TextureFormat::BC7_SRGB,
        .generateMips = true,
        .isSRGB = true
    };

    const auto result = TextureProcessor::Convert(tgaData, options);
    ASSERT_TRUE(result.success);
    EXPECT_EQ(result.width, 8u);
    EXPECT_EQ(result.height, 8u);
    EXPECT_EQ(result.format, TextureFormat::BC7_SRGB);

    // 8x8 -> 4x4 -> 2x2 -> 1x1 = 4 levels
    ASSERT_EQ(result.mips.size(), 4u);
    EXPECT_FALSE(result.payload.empty());

    // BC7 block size is 16 bytes for each 4x4 block:
    // Mip 0 (8x8): 2x2 blocks = 4 * 16 = 64 bytes
    // Mip 1 (4x4): 1x1 block  = 1 * 16 = 16 bytes
    // Mip 2 (2x2): 1x1 block  = 1 * 16 = 16 bytes (clamped to 4x4 block)
    // Mip 3 (1x1): 1x1 block  = 1 * 16 = 16 bytes (clamped to 4x4 block)
    EXPECT_EQ(result.mips[0].byteSize, 64u);
    EXPECT_EQ(result.mips[1].byteSize, 16u);
    EXPECT_EQ(result.mips[2].byteSize, 16u);
    EXPECT_EQ(result.mips[3].byteSize, 16u);
    EXPECT_EQ(result.payload.size(), 64u + 16u + 16u + 16u);
}

TEST(TextureProcessorTests, StubsReturnNotImplemented) {
    EXPECT_FALSE(TextureProcessor::BuildAtlas({}, "atlas.png", {}).success);
    EXPECT_FALSE(TextureProcessor::PackChannels({}, {}).success);
    EXPECT_FALSE(TextureProcessor::ProcessNormalMap("norm.png", {}).success);
    EXPECT_FALSE(TextureProcessor::BuildCubemap({}, {}).success);
}
