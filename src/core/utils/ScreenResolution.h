#pragma once

#include <common/Common.h>

namespace zzz::engine
{
	class ScreenResolution final
	{
	public:
		constexpr ScreenResolution(zU32 w, zU32 h) :
			Width{ w },
			Height{ h }
		{
		}

		inline zU32 GetWidth() const noexcept { return Width; }
		inline zU32 GetHeight() const noexcept { return Height; }

		private:
			zU32 Width;
			zU32 Height;
	};

	// Минимальный размер окна (клиентской области) в пикселях.
	// Win 11: минимум 148 с системной рамкой
	inline constexpr zU32 c_MinWinSize = 150;

	inline constexpr ScreenResolution c_UHD_4K{ 3840, 2160 };
	inline constexpr ScreenResolution c_UHD_8K{ 7680, 4320 };
}