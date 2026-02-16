
#pragma once

namespace zzz
{
	inline void DebugOutput(
		std::wstring_view msg,
		const std::source_location& loc = std::source_location::current())
	{
#if defined(_DEBUG)
		std::wstring output =
			L">>>>> -= DebugOutput =-"
			L"\n    Message: " + std::wstring(msg) +
			L"\n    Source: [" + std::wstring(loc.function_name(), loc.function_name() + std::strlen(loc.function_name())) +
			L"] line: " + std::to_wstring(loc.line()) +
			L", file: " + std::wstring(loc.file_name(), loc.file_name() + std::strlen(loc.file_name())) + L"\n";

#if defined(ZPLATFORM_MSWINDOWS)
		OutputDebugStringW(output.c_str());
#else
		std::wcerr << output;
#endif
#endif // _DEBUG
	}

	inline void DebugOutputLite( std::wstring_view msg)
	{
#if defined(_DEBUG)
		std::wstring output = L">>>>> DebugMessage: " + std::wstring(msg) + L"\n";
#if defined(ZPLATFORM_MSWINDOWS)
		OutputDebugStringW(output.c_str());
#else
		std::wcerr << output;
#endif
#endif // _DEBUG
	}
}
