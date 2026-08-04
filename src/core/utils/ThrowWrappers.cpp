
#include <sstream>
#include <stdexcept>
//#include <core/utils/Types.h>
#include <core/utils/Ensure.h>
#include <core/utils/Defines.h>
#include <core/utils/Macroses.h>
#include <core/utils/MemoryUtils.h>
#include <core/utils/ThrowWrappers.h>
#include <logger/logger.h>
#include "Macroses.h"

#include "ThrowWrappers.h"

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
}
