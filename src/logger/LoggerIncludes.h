#pragma once

#include <source_location>
#include <core/CoreIncludes.h>
#include <core/enums/eLogMessageType.h>
#include <core/utils/Macroses.h>

namespace zzz::logger
{
	struct LogCallbackEntry
	{
		uint64_t timestamp;
		int type;
		const char* text;
		const char* file;
		const char* function;
		uint32_t line;
	};

	typedef void (__stdcall *LogCallback)(const LogCallbackEntry& entry);
}
