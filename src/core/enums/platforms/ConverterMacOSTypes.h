#pragma once

#include <core/enums/platforms/eMacOSEnums.h>

namespace zzz::core
{

	class ConverterMacOSTypes final
	{
	public:
		ConverterMacOSTypes() = delete;

		// eMacOSWindowMode
		[[nodiscard]] static constexpr bool ToNativeFullscreen(eMacOSWindowMode mode) noexcept
		{
			return mode == eMacOSWindowMode::Fullscreen;
		}

		[[nodiscard]] static constexpr eMacOSWindowMode ToEngineWindowMode(bool isFullscreen) noexcept
		{
			return isFullscreen ? eMacOSWindowMode::Fullscreen : eMacOSWindowMode::Windowed;
		}
	};
}
