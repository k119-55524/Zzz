#pragma once

#if Z_WINDOWS

#include "../../../headers/MSWin.h"

namespace zzz
{
	const std::string wstring_to_string(const std::wstring& wstr)
	{
		if (wstr.empty())
			return {};

		const int size_needed = ::WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			wstr.data(),
			static_cast<int>(wstr.size()),
			nullptr,
			0,
			nullptr,
			nullptr);

		if (size_needed <= 0)
			THROW_RUNTIME("WideCharToMultiByte size query failed.");

		std::string result(static_cast<size_t>(size_needed), '\0');

		const int converted = ::WideCharToMultiByte(
			CP_UTF8,
			WC_ERR_INVALID_CHARS,
			wstr.data(),
			static_cast<int>(wstr.size()),
			result.data(),
			size_needed,
			nullptr,
			nullptr);

		if (converted != size_needed)
			THROW_RUNTIME("WideCharToMultiByte conversion failed.");

		return result;
	}
}
#endif