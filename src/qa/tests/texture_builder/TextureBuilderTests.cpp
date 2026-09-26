#include <gtest/gtest.h>
#include "texture_builder/TextureBuilder.h"

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

TEST(TextureBuilderTests, ProbeValidTga) {
    const TextureBuilder builder;
    const auto tgaData = CreateTestTga(16, 16);
    const auto probeResult = builder.Probe(tgaData);

    ASSERT_TRUE(probeResult.has_value());
    const auto& info = probeResult.value();
    EXPECT_EQ(info.width, 16u);
    EXPECT_EQ(info.height, 16u);
    EXPECT_EQ(info.channels, 4u);
    EXPECT_TRUE(info.hasAlpha);
    EXPECT_TRUE(info.IsDivisibleByTwo());
    EXPECT_TRUE(info.IsPowerOfTwo());
}

TEST(TextureBuilderTests, ProbeNonPowerOfTwoDivisibleByTwo) {
    const TextureBuilder builder;
    const auto tgaData = CreateTestTga(6, 10);
    const auto probeResult = builder.Probe(tgaData);

    ASSERT_TRUE(probeResult.has_value());
    const auto& info = probeResult.value();
    EXPECT_EQ(info.width, 6u);
    EXPECT_EQ(info.height, 10u);
    EXPECT_TRUE(info.IsDivisibleByTwo());
    EXPECT_FALSE(info.IsPowerOfTwo());
}

TEST(TextureBuilderTests, ProbeEmptyBufferFails) {
    const TextureBuilder builder;
    const std::vector<uint8_t> empty;
    const auto probeResult = builder.Probe(empty);
    EXPECT_FALSE(probeResult.has_value());
}

TEST(TextureBuilderTests, ConvertOddDimensionsFailsValidation) {
    const TextureBuilder builder;
    const auto oddTga = CreateTestTga(5, 5);
    TextureConvertOptions options{
        .targetFormat = zzz::core::ePixelFormat::RGBA8_UNORM,
        .generateMips = false
    };

    const auto result = builder.Convert(oddTga, options);
    EXPECT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("must be divisible by 2"), std::string::npos);
}

TEST(TextureBuilderTests, ConvertRgba8WithMips) {
    const TextureBuilder builder;
    const auto tgaData = CreateTestTga(4, 4);
    TextureConvertOptions options{
        .targetFormat = zzz::core::ePixelFormat::RGBA8_UNORM,
        .generateMips = true
    };

    const auto result = builder.Convert(tgaData, options);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->metadata.width, 4u);
    EXPECT_EQ(result->metadata.height, 4u);
    EXPECT_EQ(result->metadata.format, zzz::core::ePixelFormat::RGBA8_UNORM);

    // 4x4 -> 2x2 -> 1x1 = 3 levels
    EXPECT_EQ(result->metadata.mipCount, 3u);
    EXPECT_EQ(result->payload.size(), 64u + 16u + 4u);
}

TEST(TextureBuilderTests, ConvertBc7Compression) {
    const TextureBuilder builder;
    const auto tgaData = CreateTestTga(8, 8);
    TextureConvertOptions options{
        .targetFormat = zzz::core::ePixelFormat::BC7_SRGB,
        .generateMips = true
    };

    const auto result = builder.Convert(tgaData, options);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->metadata.width, 8u);
    EXPECT_EQ(result->metadata.height, 8u);
    EXPECT_EQ(result->metadata.format, zzz::core::ePixelFormat::BC7_SRGB);

    // 8x8 -> 4x4 -> 2x2 -> 1x1 = 4 levels
    EXPECT_EQ(result->metadata.mipCount, 4u);
    EXPECT_FALSE(result->payload.empty());
    EXPECT_EQ(result->payload.size(), 64u + 16u + 16u + 16u);
}

TEST(TextureBuilderTests, StubsReturnNotImplemented) {
    const TextureBuilder builder;
    EXPECT_FALSE(builder.BuildAtlas({}, "atlas.png", {}).has_value());
    EXPECT_FALSE(builder.PackChannels({}, {}).has_value());
    EXPECT_FALSE(builder.ProcessNormalMap("norm.png", {}).has_value());
    EXPECT_FALSE(builder.BuildCubemap({}, {}).has_value());
}
