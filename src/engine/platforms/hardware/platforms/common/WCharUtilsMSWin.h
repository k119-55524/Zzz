#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include <string>

namespace zzz::engine
{
	/**
	 * @brief Конвертирует нуль-терминированную широкую (UTF-16) строку Windows в UTF-8 std::string.
	 * Общий хелпер для MSWin-коллекторов аппаратной телеметрии (Motherboard/Storage/Network),
	 * вынесен сюда, чтобы не дублировать одну и ту же лямбду в каждом файле.
	 */
	[[nodiscard]] inline std::string WCharToUtf8MSWin(const WCHAR* wstr)
	{
		if (!wstr || !*wstr)
			return {};

		int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
		if (sizeNeeded <= 1)
			return {};

		std::string result(sizeNeeded - 1, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, result.data(), sizeNeeded, NULL, NULL);
		return result;
	}
}

#endif // defined(Z_WINDOWS)
