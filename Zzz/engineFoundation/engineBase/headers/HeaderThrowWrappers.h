#pragma once

#pragma once

namespace zzz
{
	[[noreturn]]
	inline void throw_runtime_error(
		const std::string& msg = "Throw runtime error",
		const std::source_location& loc = std::source_location::current())
	{
		throw std::runtime_error(
			"Exception: " + msg +
			". Method=" + std::string(loc.function_name()) +
			", line=" + std::to_string(loc.line()) +
			", file=" + std::string(loc.file_name()));
	}

	[[noreturn]]
	inline void throw_invalid_argument(
		const std::string& msg = "Throw invalid argument",
		const std::source_location& loc = std::source_location::current())
	{
		throw std::invalid_argument(
			"Exception: " + msg +
			". Method=" + std::string(loc.function_name()) +
			", line=" + std::to_string(loc.line()) +
			", file=" + std::string(loc.file_name()));
	}

	[[noreturn]]
	inline void throw_out_of_range(
		const std::string& msg = "Throw out of range",
		const std::source_location& loc = std::source_location::current())
	{
		throw std::out_of_range(
			"Exception: " + msg +
			". Method=" + std::string(loc.function_name()) +
			", line=" + std::to_string(loc.line()) +
			", file=" + std::string(loc.file_name()));
	}

	// Математическое переполнение
	[[noreturn]]
	inline void throw_overflow_error(
		const std::string& msg = "Throw overflow error",
		const std::source_location& loc = std::source_location::current())
	{
		throw std::overflow_error(
			"Exception: " + msg +
			". Method=" + std::string(loc.function_name()) +
			", line=" + std::to_string(loc.line()) +
			", file=" + std::string(loc.file_name()));
	}
}