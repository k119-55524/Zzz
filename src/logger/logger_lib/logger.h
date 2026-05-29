#pragma once

#include "header.h"

namespace zzz::logger
{
	class Logger
	{
	public:
		template <typename... Args>
		static void DebugOutput(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if ZADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeDebugOutputString(loc, formatted);
			DebugOutputIDE(output);
#endif
		}

		template<typename... Args>
		static void DebugOutputLite(const std::source_location& loc, std::format_string<Args...> fmt, Args&&... args)
		{
#if ZADD_LOGGER
			auto formatted = std::format(fmt, std::forward<Args>(args)...);
			auto output = MakeDebugOutputLiteString(loc, formatted);
			DebugOutputIDE(output);
#endif
		}

		static void DebugOutputLite(const std::source_location& loc, std::string_view message)
		{
#if ZADD_LOGGER
			auto output = MakeDebugOutputLiteString(loc, message.data());
			DebugOutputIDE(output);
#endif
		}

	private:
		static std::string MakeDebugOutputString(const std::source_location& loc, const std::string& msg);
		static std::string MakeDebugOutputLiteString(const std::source_location& loc, const std::string& msg);
		static void DebugOutputIDE(const std::string& output) noexcept;
	};
}
