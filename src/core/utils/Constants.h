#pragma once

#include "core/CoreIncludes.h"
#include "core/io/FileHeader.h"
#include "core/utils/LogCategory.h"

namespace zzz::core
{
#pragma region Config file constants
	constexpr std::string_view c_ConfigFileName = "user.dat";
	constexpr FileHeader<3> c_ConfigHeader
	{
		std::array<std::byte, 3>{
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z')
		}
	};
	constexpr zU8 c_ConfigFileMajorVersion = 1;
	constexpr zU8 c_ConfigFileMinorVersion = 1; // v1: добавлен opt-out список отключённых категорий логирования (UserSettingsManager)
	constexpr zU8 c_ConfigFilePatchVersion = 0;

	constexpr zU32 c_DefaultWindowWidth = 800;
	constexpr zU32 c_DefaultWindowHeight = 600;
	constexpr zzz::math::Size2D<zU32> c_DefaultWindowSize{ c_DefaultWindowWidth, c_DefaultWindowHeight };

#if Z_DESKTOP
	// Минимальный размер окна (клиентской области) в пикселях.
	constexpr zU32 c_MinWinSize = 150;
#endif // Z_DESKTOP

	constexpr zzz::math::Size2D<zU32> c_UHD_4K{ 3840, 2160 };
	constexpr zzz::math::Size2D<zU32> c_UHD_8K{ 7680, 4320 };
#pragma endregion // Config file constants

#pragma region Game Package file constants
	constexpr std::string_view c_GamePackageFileName = "assets/package.dat";
	constexpr FileHeader<3> c_GamePackageHeader
	{
		std::array<std::byte, 3>{
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('Z'),
			static_cast<std::byte>('P')
		}
	};
	constexpr zU8 c_GamePackageFileMajorVersion = 1;
	constexpr zU8 c_GamePackageFileMinorVersion = 1;
	constexpr zU8 c_GamePackageFilePatchVersion = 0;
#pragma endregion // Game Package file constants

#pragma region Microsoft Windows constants
	constexpr std::string_view c_IcoResourceName = "IDI_ICON1";
#pragma endregion // Microsoft Windows constants

#pragma region GAPI Engine Constants
	constexpr uint32_t c_FramesInFlight = 2;

#if defined(Z_D3D12)
	constexpr DXGI_FORMAT c_DefaultBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	constexpr DXGI_FORMAT c_DefaultDepthFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	constexpr D3D_FEATURE_LEVEL c_DefaultFeatureLevel = D3D_FEATURE_LEVEL_12_0;
#elif defined(Z_VULKAN)
	constexpr uint32_t c_DefaultVulkanApiVersion = VK_API_VERSION_1_4;
	constexpr VkFormat c_DefaultBackBufferFormat = VK_FORMAT_R8G8B8A8_UNORM;
	constexpr VkFormat c_DefaultDepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
#elif defined(Z_METAL)
	constexpr MTLPixelFormat c_DefaultBackBufferFormat = MTLPixelFormatRGBA8Unorm;
	constexpr MTLPixelFormat c_DefaultDepthFormat = MTLPixelFormatDepth32Float_Stencil8;
#endif
#pragma endregion // GAPI Engine Constants

#pragma region Network & Logger constants
	constexpr std::string_view c_LocalhostIPv4 = "127.0.0.1";
	constexpr zU16 c_DefaultLoggerPort = 3030;

	// Версия бинарного протокола передачи логов по сети
	constexpr zU32 c_LogProtocolVersion = 0;

#if Z_DESKTOP
	constexpr zU32 c_MaxNetworkLogQueueSize = 2000;
#elif Z_MOBILE
	constexpr zU32 c_MaxNetworkLogQueueSize = 500;
#endif
#pragma endregion // Network & Logger constants 

#pragma region Built-in Log Categories
#define Z_DECLARE_LOG_CATEGORY_ENGINE(Name) \
	inline constexpr LogCategory Name{ #Name, eLogCategoryGroup::Engine, false }

#define Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE(Name) \
	inline constexpr LogCategory Name{ #Name, eLogCategoryGroup::Engine, true }

	// Пример обязательной (GUARANTEED) категории логирования:
	// Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE(MyCriticalSystem);

	Z_DECLARE_LOG_CATEGORY_ENGINE(GAPI);
	Z_DECLARE_LOG_CATEGORY_ENGINE(ECS);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Audio);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Physics);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Assets);
	Z_DECLARE_LOG_CATEGORY_ENGINE(Network);
	Z_DECLARE_LOG_CATEGORY_ENGINE(UI);

#undef Z_DECLARE_LOG_CATEGORY_ENGINE
#undef Z_DECLARE_GUARANTEED_LOG_CATEGORY_ENGINE
#pragma endregion // Built-in Log Categories
}
