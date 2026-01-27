
#pragma once

namespace zzz
{
	[[noreturn]]
	inline void throw_runtime_error(
		const std::string& msg = "Throw runtime error",
		const std::source_location& loc = std::source_location::current())
	{
		std::ostringstream oss;
		oss << "+--------------\nException ->"
			<< "\n+-> Method = " << loc.function_name()
			<< ",\n+-> line = " << loc.line()
			<< ",\n+-> file = " << loc.file_name()
			<< "\n+-> " << msg;

		throw std::runtime_error(oss.str());
	}

	[[noreturn]]
	inline void throw_invalid_argument(
		const std::string& msg = "Throw invalid argument",
		const std::source_location& loc = std::source_location::current())
	{
		std::ostringstream oss;
		oss << "+--------------\nException ->"
			<< "\n+-> Method = " << loc.function_name()
			<< ",\n+-> line = " << loc.line()
			<< ",\n+-> file = " << loc.file_name()
			<< "\n+-> " << msg;

		throw std::invalid_argument(oss.str());
	}

	[[noreturn]]
	inline void throw_out_of_range(
		const std::string& msg = "Throw out of range",
		const std::source_location& loc = std::source_location::current())
	{
		std::ostringstream oss;
		oss << "+--------------\nException ->"
			<< "\n+-> Method = " << loc.function_name()
			<< ",\n+-> line = " << loc.line()
			<< ",\n+-> file = " << loc.file_name()
			<< "\n+-> " << msg;

		throw std::out_of_range(oss.str());
	}

	// Математическое переполнение
	[[noreturn]]
	inline void throw_overflow_error(
		const std::string& msg = "Throw overflow error",
		const std::source_location& loc = std::source_location::current())
	{
		std::ostringstream oss;
		oss << "+--------------\nException ->"
			<< "\n+-> Method = " << loc.function_name()
			<< ",\n+-> line = " << loc.line()
			<< ",\n+-> file = " << loc.file_name()
			<< "\n+-> " << msg;

		throw std::overflow_error(oss.str());
	}
}