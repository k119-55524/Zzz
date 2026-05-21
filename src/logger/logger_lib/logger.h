#pragma once

#include "header.h"

namespace zzz::logger
{
	class Logger
	{
	public:
		template <typename... Args>
		static void DebugOutput(
			const std::source_location& loc,
			std::wstring_view fmt,
			Args&&... args)
		{
#if ZZZ_ENABLE_LOGGER
			auto formatted = std::vformat(fmt, std::make_wformat_args(std::forward<Args>(args)...));
			auto output = MakeDebugOutputString(loc, formatted);
			DebugOutputRaw(output);
#endif
		}

	private:
		static std::wstring MakeDebugOutputString(const std::source_location& loc, const std::wstring& msg);
		static void DebugOutputRaw(const std::wstring& output) noexcept;
	};
}
