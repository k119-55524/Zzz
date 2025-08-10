#include "pch.h"
export module strConver;

import result;

export namespace zzz
{
#if defined(_WIN64)
	result<std::wstring> string_to_wstring(const std::string& str)
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

	//std::wstring string_to_wstring1(const std::string& str)
	//{
	//	size_t size_needed = std::mbstowcs(nullptr, str.c_str(), 0);
	//	std::wstring wstr(size_needed, 0);
	//	std::mbstowcs(&wstr[0], str.c_str(), size_needed);
	//	return wstr;
	//}

	//std::wstring string_to_wstring2(const std::string& str)
	//{
	//	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//	return converter.from_bytes(str);
	//}

	const std::string wstring_to_string(const std::wstring& wstr)
	{
		if (wstr.empty())
			return {};

		int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), nullptr, 0, nullptr, nullptr);
		std::string str(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), str.data(), size_needed, nullptr, nullptr);

		return str;
	}
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
#endif

	template<typename T> inline result<T> ConvertValue(const std::wstring& text);
	template<> inline result<std::wstring> ConvertValue<std::wstring>(const std::wstring& text) { return text; }
	template<> inline result<int> ConvertValue<int>(const std::wstring& text)
	{
		try
		{
			return std::stoi(text);
		}
		catch (...)
		{
			return Unexpected(eResult::invalid_format, L"Invalid int: " + text);
		}
	}
	template<> inline result<float> ConvertValue<float>(const std::wstring& text)
	{
		try
		{
			return std::stof(text);
		}
		catch (...)
		{
			return Unexpected(eResult::invalid_format, L"Invalid float: " + text);
		}
	}
	template<> inline result<bool> ConvertValue<bool>(const std::wstring& text)
	{
		if (text == L"true" || text == L"1") return true;
		if (text == L"false" || text == L"0") return false;
		return Unexpected(eResult::invalid_format, L"Invalid bool: " + text);
	}
}