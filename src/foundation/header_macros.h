#pragma once

#define DOut(...) ::zzz::logger::Logger::LogMessage(std::source_location::current(), __VA_ARGS__)
#define DOutWarning(...) ::zzz::logger::Logger::LogWarning(std::source_location::current(), __VA_ARGS__)
#define DOutError(...) ::zzz::logger::Logger::LogError(std::source_location::current(), __VA_ARGS__)
#define DOutException(...) ::zzz::logger::Logger::LogException(std::source_location::current(), __VA_ARGS__)
#define DOutCritical(...) ::zzz::logger::Logger::LogCritical(std::source_location::current(), __VA_ARGS__)

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
	ClassName(ClassName&&) = delete; \
	ClassName& operator=(ClassName&&) = delete
