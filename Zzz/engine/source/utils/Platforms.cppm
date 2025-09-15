
#include "pch.h"
#include "..\..\Headers\PlatformsDefines.h"

export module Platforms;

export namespace zzz
{
	export class Platforms
	{
	public:
		static const size_t ArrayAligment = 32;

		enum class PlatformType
		{
			Windows,
			XboxSeries,
			PlayStation5,
			NintendoSwitch,
			Android,
			Linux,
			MacOS,
			iOS
		};

		// Получить текущую платформу в рантайме
		static constexpr PlatformType GetCurrent() noexcept
		{
#if defined(PLATFORM_WINDOWS)
			return PlatformType::Windows;
#elif defined(PLATFORM_XBOX)
			return PlatformType::XboxSeries;
#elif defined(PLATFORM_PLAYSTATION)
			return PlatformType::PlayStation5;
#elif defined(PLATFORM_SWITCH)
			return PlatformType::NintendoSwitch;
#elif defined(PLATFORM_ANDROID)
			return PlatformType::Android;
#elif defined(PLATFORM_LINUX)
			return PlatformType::Linux;
#elif defined(PLATFORM_MACOS)
			return PlatformType::MacOS;
#elif defined(PLATFORM_IOS)
			return PlatformType::iOS;
#else
			#error ">>>>> [Platforms.GetCurrent()]. Compile error. Unknown platform!""
#endif
		}

		static const char* GetName(PlatformType platform) noexcept
		{
			switch (platform)
			{
				case PlatformType::Windows:         return "Windows";
				case PlatformType::XboxSeries:      return "Xbox Series X|S";
				case PlatformType::PlayStation5:    return "PlayStation 5";
				case PlatformType::NintendoSwitch:  return "Nintendo Switch";
				case PlatformType::Android:         return "Android";
				case PlatformType::Linux:           return "Linux";
				case PlatformType::MacOS:           return "macOS";
				case PlatformType::iOS:             return "iOS";
			}

			throw_invalid_argument(">>>>> [Platforms.GetName( ... )]. Unreachable! Did you forget to handle a PlatformType case?");
		}

		static const char* GetCurrentName() noexcept { return GetName(GetCurrent()); }
	};
}