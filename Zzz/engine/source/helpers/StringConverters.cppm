#include "pch.h"
export module StringConverters;

import result;

using namespace zzz::result;

export namespace zzz
{
#ifdef _WIN64
	zResult<std::wstring> string_to_wstring(const std::string& str)
	{
		if (str.empty())
			return std::wstring();

		// Определяем размер необходимого буфера для широкой строки
		int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
		if (size_needed == 0)
			return Unexpected(eResult::failure, L">>>>> [to_wstring( ... )]. MultiByteToWideChar failed.");

		// Выделяем буфер и выполняем преобразование
		std::wstring wstr(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);

		return wstr;
	}

	const std::string wstring_to_string(const std::wstring& wstr)
	{
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
		std::string str(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &str[0], size_needed, NULL, NULL);

		return str;
	}
#endif
}