#pragma once

#include <core/enums/platforms/eLinuxEnums.h>

namespace zzz::core
{
	using namespace zzz::core;

	class ConverterLinuxTypes final
	{
	public:
		ConverterLinuxTypes() = delete;

		// eLinuxWindowMode
		[[nodiscard]] static constexpr bool ToNativeFullscreen(eLinuxWindowMode mode) noexcept
		{
			return mode == eLinuxWindowMode::Fullscreen;
		}

		[[nodiscard]] static constexpr eLinuxWindowMode ToEngineWindowMode(bool isFullscreen) noexcept
		{
			return isFullscreen ? eLinuxWindowMode::Fullscreen : eLinuxWindowMode::Windowed;
		}
	};
}
