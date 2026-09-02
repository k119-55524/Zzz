#pragma once

#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

#include "core/enums/platforms/eAndroidEnums.h"

namespace zzz::core
{

	class ConverterAndroidTypes final
	{
	public:
		ConverterAndroidTypes() = delete;

		// eAndroidScreenOrientation (Соответствие ActivityInfo.SCREEN_ORIENTATION_*)
		[[nodiscard]] static constexpr int ToNative(eAndroidScreenOrientation orientation) noexcept
		{
			switch (orientation)
			{
			case eAndroidScreenOrientation::Sensor:        return 4;  // SCREEN_ORIENTATION_SENSOR
			case eAndroidScreenOrientation::Portrait:      return 1;  // SCREEN_ORIENTATION_PORTRAIT
			case eAndroidScreenOrientation::LandscapeLeft: return 0;  // SCREEN_ORIENTATION_LANDSCAPE
			case eAndroidScreenOrientation::LandscapeRight:return 8;  // SCREEN_ORIENTATION_REVERSE_LANDSCAPE
			}
			return 0;
		}

		[[nodiscard]] static constexpr eAndroidScreenOrientation ToEngineOrientation(int nativeOrientation) noexcept
		{
			switch (nativeOrientation)
			{
			case 1: return eAndroidScreenOrientation::Portrait;
			case 0: return eAndroidScreenOrientation::LandscapeLeft;
			case 8: return eAndroidScreenOrientation::LandscapeRight;
			default: return eAndroidScreenOrientation::Sensor;
			}
		}

		// eAndroidCutoutMode (Соответствие LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_*)
		[[nodiscard]] static constexpr int ToNative(eAndroidCutoutMode cutoutMode) noexcept
		{
			switch (cutoutMode)
			{
			case eAndroidCutoutMode::Default:    return 0; // LAYOUT_IN_DISPLAY_CUTOUT_MODE_DEFAULT
			case eAndroidCutoutMode::ShortEdges: return 1; // LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES
			case eAndroidCutoutMode::Never:      return 2; // LAYOUT_IN_DISPLAY_CUTOUT_MODE_NEVER
			}
			return 0;
		}

		[[nodiscard]] static constexpr eAndroidCutoutMode ToEngineCutoutMode(int nativeCutoutMode) noexcept
		{
			switch (nativeCutoutMode)
			{
			case 1:  return eAndroidCutoutMode::ShortEdges;
			case 2:  return eAndroidCutoutMode::Never;
			default: return eAndroidCutoutMode::Default;
			}
		}
	};
}

#endif // defined(Z_ANDROID)
