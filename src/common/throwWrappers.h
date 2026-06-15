#pragma once

#include <string>
#include <sstream>
#include <source_location>
#include <stdexcept>
#include <logger/logger_lib/logger.h>
#include <common/core_macros.h>

namespace zzz
{
	[[noreturn]]
	inline void throw_runtime_error(
		const std::string& msg = "Throw runtime error",
		const std::source_location& loc = std::source_location::current())
	{
		std::ostringstream oss;
		oss << "\n+-> " << msg
			<< "\n+-> Method: " << loc.function_name()
			<< ",\n+-> line: " << loc.line()
			<< ",\n+-> file: " << loc.file_name()
			<< "\n";

		DOutException("{}", oss.str());
		throw std::runtime_error(oss.str());
	}
}