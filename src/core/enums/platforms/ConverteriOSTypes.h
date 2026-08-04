#pragma once

#include <core/enums/platforms/eiOSEnums.h>

namespace zzz::core
{
	using namespace zzz::core;

	class ConverteriOSTypes final
	{
	public:
		ConverteriOSTypes() = delete;

		// eiOSScreenOrientation (Соответствие UIInterfaceOrientationMask)
		[[nodiscard]] static constexpr unsigned int ToNative(eiOSScreenOrientation orientation) noexcept
		{
			switch (orientation)
			{
			case eiOSScreenOrientation::Portrait:       return (1 << 1); // UIInterfaceOrientationMaskPortrait
			case eiOSScreenOrientation::LandscapeLeft:  return (1 << 4); // UIInterfaceOrientationMaskLandscapeLeft
			case eiOSScreenOrientation::LandscapeRight: return (1 << 3); // UIInterfaceOrientationMaskLandscapeRight
			case eiOSScreenOrientation::AutoRotate:     return 30;       // UIInterfaceOrientationMaskAll
			}
			return 30;
		}

		[[nodiscard]] static constexpr eiOSScreenOrientation ToEngineOrientation(long nativeOrientation) noexcept
		{
			switch (nativeOrientation)
			{
			case 1: return eiOSScreenOrientation::Portrait;
			case 4: return eiOSScreenOrientation::LandscapeLeft;
			case 3: return eiOSScreenOrientation::LandscapeRight;
			default: return eiOSScreenOrientation::AutoRotate;
			}
		}

		// eiOSHomeIndicatorMode
		[[nodiscard]] static constexpr bool ToNativeHomeIndicatorAutoHidden(eiOSHomeIndicatorMode mode) noexcept
		{
			return mode == eiOSHomeIndicatorMode::AutoHidden;
		}

		[[nodiscard]] static constexpr eiOSHomeIndicatorMode ToEngineHomeIndicatorMode(bool isAutoHidden) noexcept
		{
			return isAutoHidden ? eiOSHomeIndicatorMode::AutoHidden : eiOSHomeIndicatorMode::Visible;
		}
	};
}
