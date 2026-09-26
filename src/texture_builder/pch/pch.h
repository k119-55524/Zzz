#pragma once

/**
 * @file pch.h
 * @brief Precompiled header for texture_builder library.
 */
#include <span>
#include <array>
#include <string>
#include <vector>
#include <format>
#include <cstdint>
#include <cstddef>
#include <expected>
#include <algorithm>
#include <filesystem>

#include "math/utils/Types.h"
#include "core/utils/Defines.h"

#if Z_WINDOWS
#include <wrl/client.h>
#endif

#if Z_DESKTOP
#include <DirectXTex.h>
#endif

#define STBI_NO_STDIO
#include "stb_image.h"
#include "stb_image_resize2.h"

#include "core/logger/logger.h"
#include "core/constants/LogCategoryConstants.h"
