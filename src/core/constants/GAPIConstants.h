#pragma once

/**
 * @file GAPIConstants.h
 * @brief Константы графических API (Direct3D 12, Vulkan, Metal).
 *
 * @details Определяет базовые параметры пайплайна рендеринга (число буферов в кадре),
 *          стандартные форматы бэкбуфера, буфера глубины/трафарета и уровни поддержки API.
 *
 * @note Используется в:
 *       - Graphics Context & Swapchain (инициализация фреймбуферов и форматов)
 *       - RenderPipeline / RenderPass (создание текстур рендеринга и таргетов глубины)
 *       - VulkanAPI, DirectX12API, MetalAPI
 */

#include "core/CoreIncludes.h"

namespace zzz::core
{
#pragma region GAPI Engine Constants
	/// Количество кадров в обработке (frames in flight / swapchain image count)
	constexpr uint32_t c_FramesInFlight = 2;

	/// Аппаратное выравнивание константных буферов CBV (DirectX 12 / Vulkan / Metal)
	constexpr uint32_t c_ConstantBufferAlignment = 256;

#if defined(Z_D3D12)
	/// Формат бэкбуфера Direct3D 12 по умолчанию
	constexpr DXGI_FORMAT c_DefaultBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	/// Формат буфера глубины и трафарета Direct3D 12 по умолчанию
	constexpr DXGI_FORMAT c_DefaultDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	/// Минимальный требуемый уровень возможностей D3D12
	constexpr D3D_FEATURE_LEVEL c_DefaultFeatureLevel = D3D_FEATURE_LEVEL_12_0;
#elif defined(Z_VULKAN)
	/// Требуемая версия Vulkan API по умолчанию (Vulkan 1.4)
	constexpr uint32_t c_DefaultVulkanApiVersion = VK_API_VERSION_1_4;
	/// Формат бэкбуфера Vulkan по умолчанию
	constexpr VkFormat c_DefaultBackBufferFormat = VK_FORMAT_R8G8B8A8_UNORM;
	/// Формат буфера глубины и трафарета Vulkan по умолчанию
	constexpr VkFormat c_DefaultDepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
#elif defined(Z_METAL)
	/// Формат бэкбуфера Metal по умолчанию
	constexpr MTLPixelFormat c_DefaultBackBufferFormat = MTLPixelFormatRGBA8Unorm;
	/// Формат буфера глубины и трафарета Metal по умолчанию
	constexpr MTLPixelFormat c_DefaultDepthFormat = MTLPixelFormatDepth32Float_Stencil8;
#endif
#pragma endregion // GAPI Engine Constants
}
