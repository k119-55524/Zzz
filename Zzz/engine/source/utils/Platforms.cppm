
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
#if defined(ZPLATFORM_MSWINDOWS)
			return PlatformType::Windows;
#elif defined(ZPLATFORM_XBOX)
			return PlatformType::XboxSeries;
#elif defined(ZPLATFORM_PLAYSTATION)
			return PlatformType::PlayStation5;
#elif defined(ZPLATFORM_NINTENDO_SWITCH)
			return PlatformType::NintendoSwitch;
#elif defined(ZPLATFORM_ANDROID)
			return PlatformType::Android;
#elif defined(ZPLATFORM_LINUX)
			return PlatformType::Linux;
#elif defined(ZPLATFORM_MACOS)
			return PlatformType::MacOS;
#elif defined(ZPLATFORM_IOS)
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