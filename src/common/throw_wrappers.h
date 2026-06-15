#pragma once

#include <string>
#include <source_location>

namespace zzz::common
{
	[[noreturn]] void throw_runtime_error(
		const std::string& msg = "Throw runtime error",
		const std::source_location& loc = std::source_location::current());
}