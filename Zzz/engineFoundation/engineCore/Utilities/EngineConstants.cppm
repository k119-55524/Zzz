
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
	inline constexpr std::string_view g_PipelineCachePath = "data/pso.bin"; // Путь и имя файла для сохранения кэша PSO

	// Ограничения размера окна
	inline constexpr zU32 g_MinimumWindowWidth = g_MinimumWindowWidthAndHeight;
	inline constexpr zU32 g_MinimumWindowHeight = g_MinimumWindowWidthAndHeight;
	inline constexpr zU32 g_MaximumWindowWidth = StandardScreenResolutions::UHD_4K.Width;
	inline constexpr zU32 g_MaximumWindowHeight = StandardScreenResolutions::UHD_4K.Height;

	// TODO: Для DirectX12 пока не имплементировал. Отражается только в Vulkan
	// Descriptor Pool / Heap sizes.
	inline constexpr uint32_t g_MaxUniformBuffers = 64;
	inline constexpr uint32_t g_MaxCombinedImageSamplers = 256;
	inline constexpr uint32_t g_MaxStorageBuffers = 64;
	inline constexpr uint32_t g_MaxStorageImages = 32;
	inline constexpr uint32_t g_MaxDescriptorSets = 128;

	// Количество буферов в SwapChain (Back Buffers).
	inline constexpr zU32 BACK_BUFFER_COUNT = 2;

	// Количество кадров, которые CPU может подготовить «вперед» без ожидания GPU.
	// [2] (Рекомендуется): Баланс между задержкой ввода и стабильностью. 
	// [1] : Минимальный input lag, но GPU будет простаивать, если CPU задержится.
	// [3] : Максимальная загрузка GPU, но увеличивается задержка ввода (input lag).
	inline constexpr zU32 FRAMES_IN_FLIGHT = 3;

#if defined(ZRENDER_API_D3D12)
	inline constexpr DXGI_FORMAT BACK_BUFFER_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
	inline constexpr DXGI_FORMAT DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;
#elif defined(ZRENDER_API_VULKAN)
	// Максимальная версия Vulkan, которую движок запрашивает при создании instance.
	// VK_API_VERSION_1_4 = 1.4.x (мажорная.минорная.патч).
	// Драйвер автоматически вернет максимальную поддерживаемую версию <= этой.
	// [VK_API_VERSION_1_0] : Максимальная совместимость, но нет современных фич.
	// [VK_API_VERSION_1_2] : Базовый набор: timeline semaphores, buffer device address.
	// [VK_API_VERSION_1_3] : Рекомендуется: dynamic rendering, sync2, maintenance4.
	// [VK_API_VERSION_1_4] : Текущий максимум: descriptor indexing improvements и др.
	inline constexpr uint32_t VULKAN_ENGINE_MIN_VERSION = VK_API_VERSION_1_4;

	// TODO: расмотреть вынос в файл параметров инициализации
	// ПРЕДПОЧТИТЕЛЬНЫЕ ФОРМАТЫ ЦВЕТОВОГО БУФЕРА (SwapChain / Render Target)
	// Порядок в векторе = приоритет выбора при создании surface.
	inline const std::vector<VkFormat> PREFERRED_FORMATS =
	{
		// Аппаратно-оптимизирован на большинстве GPU (особенно Intel/AMD).
		// BGR-порядок соответствует памяти дисплея -> меньше конверсий при Present.
		// SRGB-гамма: автоматическая коррекция при записи/чтении (важно для PBR).
		VK_FORMAT_B8G8R8A8_SRGB,

		// Канонический формат, поддерживается везде.
		// Может требовать swizzle (перестановку каналов) на некоторых платформах.
		// Чуть выше расход памяти при align, но предсказуемое поведение.
		VK_FORMAT_R8G8B8A8_SRGB,

		// Используются как fallback, если SRGB не поддерживается поверхностью.
		// ВНИМАНИЕ: При использовании UNORM нужно вручную делать gamma-encode в шейдере!
		// Лучше избегать для основного рендер-таргета, если нет специфических причин.
		VK_FORMAT_B8G8R8A8_UNORM,

		// Практически универсальный формат,
		// поддерживается почти всеми Vulkan-устройствами
		// и часто используется на мобильных GPU.
		// Используется если BGR-вариант не поддерживается surface.
		VK_FORMAT_R8G8B8A8_UNORM
	};

	// ПРЕДПОЧТИТЕЛЬНЫЕ ФОРМАТЫ БУФЕРА ГЛУБИНЫ (Depth/Stencil)
	// Порядок = приоритет. Движок выберет первый поддерживаемый формат.
	inline const std::vector<VkFormat> PREFERRED_DEPTH_FORMATS =
	{
		// 32-bit float depth + 8-bit stencil -> максимальная точность + stencil.
		// Критично для: shadow mapping, deferred shading, сложных пост-эффектов.
		// Минус: занимает больше памяти, может быть медленнее на мобильных GPU.
		VK_FORMAT_D32_SFLOAT_S8_UINT,

		// 24-bit depth + 8-bit stencil. Хорош для мобильных/low-end.
		// Меньше точность -> возможен z-fighting на больших дистанциях.
		// Раскомментировать, если нужна поддержка старых/мобильных устройств:
		// VK_FORMAT_D24_UNORM_S8_UINT,

		// Только глубина, 32-bit float.
		// Используется как fallback, если stencil не нужен (например, forward-рендер без теней).
		// Экономит ~25% памяти буфера глубины по сравнению с D32_S8.
		//VK_FORMAT_D32_SFLOAT
	};
#endif

	//////////////////////////////////////////////////////
	// Compile-time проверки
	//////////////////////////////////////////////////////
	static_assert(g_MinimumWindowWidth < g_MaximumWindowWidth);
	static_assert(g_MinimumWindowWidth >= g_MinimumWindowWidthAndHeight, "Minimum window width must be at least 150");
	static_assert(g_MinimumWindowHeight < g_MaximumWindowHeight);
	static_assert(g_MinimumWindowHeight >= g_MinimumWindowWidthAndHeight, "Minimum window height must be at least 150");
	static_assert(BACK_BUFFER_COUNT >= 2, "At least 2 back buffers required");
	static_assert(FRAMES_IN_FLIGHT >= 2, "At least 2 frames in flight required");
}