#pragma once

#include <format>
#include <source_location>
#include <string>
#include "Export.h"

#ifndef THROW_RUNTIME
#define THROW_RUNTIME(...) ::zzz::core::throw_runtime_error(std::format(__VA_ARGS__), std::source_location::current())
#endif

namespace zzz::core
{
	[[noreturn]] Z_CORE_API void throw_runtime_error(
		const std::string& msg = "Throw runtime error",
		const std::source_location& loc = std::source_location::current());

	[[noreturn]] Z_CORE_API void throw_ensure(
		std::string_view message,
		const std::source_location& loc = std::source_location::current());
}