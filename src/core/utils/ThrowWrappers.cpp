
#include <sstream>
#include <stdexcept>

#include "core/utils/Macroses.h"

namespace zzz::core
{
	[[noreturn]] Z_CORE_API void throw_runtime_error(
		const std::string& msg,
		const std::source_location& loc)
	{
		std::ostringstream oss;
		oss << "\n+-> " << msg
			<< "\n+-> Метод: " << loc.function_name()
			<< ",\n+-> строка: " << loc.line()
			<< ",\n+-> файл: " << loc.file_name()
			<< "\n";

		DOutException("{}", oss.str());
		throw std::runtime_error(oss.str());
	}

	[[noreturn]] Z_CORE_API void throw_ensure(
		std::string_view message,
		const std::source_location& loc)
	{
		std::ostringstream oss;
		oss << "\n+-> [ENSURE FAILED]: " << message
			<< "\n+-> Метод: " << loc.function_name()
			<< ",\n+-> строка: " << loc.line()
			<< ",\n+-> файл: " << loc.file_name()
			<< "\n";

		DOutException("{}", oss.str());
		throw std::runtime_error(oss.str());
	}
}
