#pragma once

#include <format>
#include <expected>

#define DOut(...) ::zzz::logger::Logger::LogMessage(std::source_location::current(), __VA_ARGS__)
#define DOutWarning(...) ::zzz::logger::Logger::LogWarning(std::source_location::current(), __VA_ARGS__)
#define DOutError(...) ::zzz::logger::Logger::LogError(std::source_location::current(), __VA_ARGS__)
#define DOutException(...) ::zzz::logger::Logger::LogException(std::source_location::current(), __VA_ARGS__)
#define DOutCritical(...) ::zzz::logger::Logger::LogCritical(std::source_location::current(), __VA_ARGS__)

#define THROW_RUNTIME(...) ::zzz::throw_runtime_error(std::format(__VA_ARGS__), std::source_location::current())
#define THROW_INVALID_ARGUMENT(...) ::zzz::throw_invalid_argument(std::format(__VA_ARGS__), std::source_location::current())
#define THROW_OUT_OF_RANGE(...) ::zzz::throw_out_of_range(std::format(__VA_ARGS__), std::source_location::current())
#define THROW_OVERFLOW(...) ::zzz::throw_overflow_error(std::format(__VA_ARGS__), std::source_location::current())

/// @brief Макрос для возврата std::unexpected с логированием ошибки.
#define UNEXPECTED(fmt, ...) \
    ([&]() { \
        auto msg = std::format(fmt, ##__VA_ARGS__); \
        DOutError("{}", msg); \
        return std::unexpected(std::move(msg)); \
    }())

/// @brief Запрещает копирование класса.
#define Z_NO_COPY(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete

/// @brief Запрещает перемещение класса.
#define Z_NO_MOVE(ClassName) \
	ClassName(ClassName&&) noexcept = delete; \
	ClassName& operator=(ClassName&&) noexcept = delete

/// @brief Запрещает копирование и перемещение класса.
#define Z_NO_COPY_MOVE(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName& operator=(const ClassName&) = delete; \
	ClassName(ClassName&&) = delete; \
	ClassName& operator=(ClassName&&) = delete
