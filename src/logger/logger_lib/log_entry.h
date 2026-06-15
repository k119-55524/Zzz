#pragma once

#include "header.h"

#if Z_ADD_LOGGER || Z_DEVELOPMENT_BUILD

namespace zzz::logger
{
	struct LogEntry
	{
		uint64_t timestamp;
		zzz::common::eLogMessageType type;
		std::string text;
		std::string file;
		std::string function;
		uint32_t line;
	};
}

#endif
