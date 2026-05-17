#pragma once

#include <source_location>

#define DOut(...) \
	::zzz::logger::Logger::DebugOutput( \
		std::source_location::current(), \
		__VA_ARGS__)