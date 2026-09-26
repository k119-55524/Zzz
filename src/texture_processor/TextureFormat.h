#pragma once

#include <cstdint>

namespace zzz::texture {

enum class TextureFormat : uint32_t {
    Unknown = 0,
    RGBA8_UNORM = 1,
    RGBA8_SRGB = 2,
    BC1_UNORM = 3,
    BC1_SRGB = 4,
    BC3_UNORM = 5,
    BC3_SRGB = 6,
    BC4_UNORM = 7,
    BC4_SNORM = 8,
    BC5_UNORM = 9,
    BC5_SNORM = 10,
    BC7_UNORM = 11,
    BC7_SRGB = 12
};

} // namespace zzz::texture
