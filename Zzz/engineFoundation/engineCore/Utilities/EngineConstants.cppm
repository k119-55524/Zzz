export module EngineConstants;
import Version;
import ScreenResolution;
using namespace zzz::core;

namespace zzz
{
	inline constexpr zU32 g_MinimumWindowWidthAndHeight = 150;   // Win 11: минимум 148 с системной рамкой
}

export namespace zzz
{
	inline constexpr std::string_view g_EngineName = "zEngine";
	inline constexpr Version g_EngineVersion{ 0, 0, 1 };

	// Ограничения размера окна
	inline constexpr zU32 g_MinimumWindowWidth = g_MinimumWindowWidthAndHeight;
	inline constexpr zU32 g_MinimumWindowHeight = g_MinimumWindowWidthAndHeight;
	inline constexpr zU32 g_MaximumWindowWidth = StandardScreenResolutions::UHD_4K.Width;
	inline constexpr zU32 g_MaximumWindowHeight = StandardScreenResolutions::UHD_4K.Height;

	// Количество задних буферов для свопчейна (двойная буферизация)
	inline constexpr zU32 BACK_BUFFER_COUNT = 2;

#if defined(ZRENDER_API_VULKAN)
	// Максимальная поддерживаемая версия Vulkan
	inline constexpr uint32_t VULKAN_ENGINE_MAX_VERSION = VK_API_VERSION_1_4;
#endif

	//////////////////////////////////////////////////////
	// Compile-time проверки
	//////////////////////////////////////////////////////
	static_assert(g_MinimumWindowWidth < g_MaximumWindowWidth);
	static_assert(g_MinimumWindowWidth >= g_MinimumWindowWidthAndHeight, "Minimum window width must be at least 150");
	static_assert(g_MinimumWindowHeight < g_MaximumWindowHeight);
	static_assert(g_MinimumWindowHeight >= g_MinimumWindowWidthAndHeight, "Minimum window height must be at least 150");
	static_assert(BACK_BUFFER_COUNT >= 2, "At least 2 back buffers required");
}