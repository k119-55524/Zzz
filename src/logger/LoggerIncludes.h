#pragma once

#include <source_location>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <condition_variable>

#include <core/CoreIncludes.h>
#include <core/enums/eLogMessageType.h>
#include <core/utils/Macroses.h>
#include <core/utils/Constants.h>
#include <core/templates/DoubleBufferedVector.h>

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
