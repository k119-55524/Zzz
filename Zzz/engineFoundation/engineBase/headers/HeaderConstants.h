#pragma once

namespace zzz
{
	//-----------------------------------
	// Общие константы для всех проектов
	//-----------------------------------
	const zU32 c_MinimumWindowsWidth = 150;		// TODO: Win 11 ограничение по минимальному размеру окна 148 при использовании системной рамки
	const zU32 c_MinimumWindowsHeight = 150;
	const zU32 c_MaximumWindowsWidth = 3840;	// Ultra HD 4K
	const zU32 c_MaximumWindowsHeight = 2160;	// Ultra HD 4K

	static constexpr zU32 BACK_BUFFER_COUNT = 2;
}