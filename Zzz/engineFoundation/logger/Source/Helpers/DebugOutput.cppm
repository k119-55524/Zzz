
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
		DebugOutputRaw(std::wstring(msg) + L"\n");
#endif
	}

	export template<typename First, typename... Rest>
	inline void DebugOutputLite(std::wstring_view fmt, First&& first, Rest&&... rest)
	{
#if defined(_DEBUG)
		std::wstring result;
		std::wstring_view f = fmt;
		size_t argIndex = 0;

		auto appendNext = [&](auto&& val)
			{
				std::wstring replacement;
				using T = std::decay_t<decltype(val)>;
				if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, std::wstring_view>)
					replacement = val;
				else if constexpr (std::is_same_v<T, const wchar_t*>)
					replacement = val;
				else if constexpr (std::is_arithmetic_v<T>)
					replacement = std::to_wstring(val);
				else
					static_assert(sizeof(T) == 0, "Unsupported type for DebugOutputLiteFmt");

				size_t pos = result.find(L"{}");
				if (pos != std::wstring::npos)
					result.replace(pos, 2, replacement);
				else
					result += replacement; // если лишние аргументы
			};

		// скопируем формат в result для редактирования
		result = fmt;

		(appendNext(std::forward<First>(first)), ..., appendNext(std::forward<Rest>(rest)));

		DebugOutputRaw(result + L"\n");
#endif
	}
}