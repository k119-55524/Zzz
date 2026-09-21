#pragma once

#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <source_location>
#include <condition_variable>
#include <core/CoreIncludes.h>
#include <core/utils/Macroses.h>
#include <core/constants/Constants.h>
#include <core/serialize/Serializer.h>
#include <core/enums/eLogMessageType.h>
#include <core/templates/DoubleBufferedVector.h>

using namespace zzz::core;

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
