
#include <sstream>
#include <stdexcept>
//#include <common/common.h>
#include <logger/logger.h>
#include "macroses.h"

#include "throw_wrappers.h"

namespace zzz::common
{
	[[noreturn]] Z_COMMON_API void throw_runtime_error(
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
}
