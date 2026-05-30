#pragma once

#include "header.h"

#define DOut(...) ::zzz::logger::Logger::DebugOutput(std::source_location::current(), __VA_ARGS__)
#define DOutLite(...) ::zzz::logger::Logger::DebugOutputLite(std::source_location::current(), __VA_ARGS__)

/// @brief Макрос для возврата std::unexpected с логированием ошибки.
#define UNEXPECTED(...) \
	do { \
		auto msg = std::format(__VA_ARGS__); \
		DOutLite(msg); \
		return std::unexpected(std::move(msg)); \
	} while(false)

/// @brief Запрещает копирование класса.
#define Z_NO_COPY(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete

/// @brief Запрещает перемещение класса.
#define Z_NO_MOVE(ClassName) \
	ClassName(ClassName&&) = delete; \
	ClassName& operator=(ClassName&&) = delete
