#pragma once

// C++ Standard Library Includes
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iostream>
#include <variant>
#include <optional>
#include <stdexcept>
#include <system_error>
#include <format>
#include <expected>
#include <bitset>
#include <concepts>

// Core Defines
#include "core/utils/Defines.h"
#include "core/utils/Types.h"
#include "core/utils/NativeAppData.h"
#include <math/Math.h>

// Platform Headers
#include "core/headers/Apple.h"
#include "core/headers/MSWin.h"
#include "core/headers/Linux.h"
#include "core/headers/Android.h"
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4062)
#endif
#include "core/headers/DirectX12.h"
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
#include "core/headers/Vulkan.h"
#include "core/headers/Metal.h"
