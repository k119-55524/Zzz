
export module EngineConstants;

import Version;

using namespace zzz::core;

export namespace zzz
{
	inline constexpr std::wstring_view g_EngineName = L"zEngine";
	inline constexpr Version g_EngineVersion{ 0, 0, 1 };

	inline constexpr zU32 g_MinimumWindowsWidth = 150;		// TODO: Win 11 ограничение по минимальному размеру окна 148 при использовании системной рамки
	inline constexpr zU32 g_MinimumWindowsHeight = 150;
	inline constexpr zU32 g_MaximumWindowsWidth = 3840;		// Ultra HD 4K
	inline constexpr zU32 g_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	inline constexpr zU32 BACK_BUFFER_COUNT = 2;
	inline constexpr zU32 MAX_FRAMES_IN_FLIGHT = 2;
}