
export module DebugOutput;

namespace zzz::logger
{
	inline void DebugOutputRaw(const std::wstring& output) noexcept
	{
#if defined(ZPLATFORM_MSWINDOWS)
		OutputDebugStringW(output.c_str());
#else
		std::wcerr << output << std::endl;
#endif
	}

	inline std::wstring MakeDebugOutputString(
		const std::source_location& loc,
		const std::wstring& msg)
	{
		return
			L">>>>> -= DebugOutput =-\n"
			L"    Message: " + msg +
			L"\n    Source: [" +
			std::wstring(loc.function_name(),
				loc.function_name() + std::strlen(loc.function_name())) +
			L"] line: " + std::to_wstring(loc.line()) +
			L", file: " +
			std::wstring(loc.file_name(),
				loc.file_name() + std::strlen(loc.file_name())) +
			L"\n";
	}

	template<typename... Args>
	inline void DebugOutputImpl(
		const std::source_location& loc,
		std::wstring_view fmt,
		Args&&... args)
	{
#if defined(_DEBUG)
		auto formatted = std::format(fmt, std::forward<Args>(args)...);
		auto output = MakeDebugOutputString(loc, formatted);
		DebugOutputRaw(output);
#endif // _DEBUG
	}

	export template <typename... Args>
		inline void DebugOutput(
			const std::source_location& loc,
			std::wstring_view fmt,
			Args&&... args)
	{
#if defined(_DEBUG)
		auto formatted = std::vformat(fmt, std::make_wformat_args(std::forward<Args>(args)...));
		auto output = MakeDebugOutputString(loc, formatted);
		DebugOutputRaw(output);
#endif
	}

	export inline void DebugOutputLite(std::wstring_view msg)
	{
#if defined(_DEBUG)
		std::wstring output = L">>>>> DebugMessage: " + std::wstring(msg) + L"\n";
		DebugOutputRaw(output);
#endif // _DEBUG
	}
}