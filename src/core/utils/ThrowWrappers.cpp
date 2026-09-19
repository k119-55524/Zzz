
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
		oss << "\n>>>>> [Runtime Error]\n"
			<< "    +-> " << msg << "\n"
			<< "    +-> Метод: " << loc.function_name() << "\n"
			<< "    +-> Место: " << loc.file_name() << ":" << loc.line();

		DOutException("{}", oss.str());
		throw std::runtime_error(oss.str());
	}

	[[noreturn]] Z_CORE_API void throw_ensure(
		std::string_view message,
		const std::source_location& loc)
	{
		std::ostringstream oss;
		oss << "\n>>>>> [Ensure]\n"
			<< "    +-> " << message << "\n"
			<< "    +-> Метод: " << loc.function_name() << "\n"
			<< "    +-> Место: " << loc.file_name() << ":" << loc.line();

		DOutException("{}", oss.str());
		throw std::runtime_error(oss.str());
	}
}
