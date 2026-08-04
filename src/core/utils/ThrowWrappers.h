#pragma once

#include <string>
#include <source_location>
#include "Export.h"

namespace zzz::common
{
	[[noreturn]] Z_CORE_API void throw_runtime_error(
		const std::string& msg = "Throw runtime error",
		const std::source_location& loc = std::source_location::current());
}