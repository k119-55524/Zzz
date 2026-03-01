#pragma once

// ћакрос дл€ запрета копировани€ и перемещени€ класса
#define Z_NO_COPY_MOVE(ClassName) \
	private: \
		ClassName(const ClassName&) = delete; \
		ClassName(ClassName&&) = delete; \
		ClassName& operator=(const ClassName&) = delete; \
		ClassName& operator=(ClassName&&) = delete;

// ћакрос дл€ запрета создани€ с конструктором без параметров, копировани€ и перемещени€ класса
#define Z_NO_CREATE_COPY(ClassName) \
		ClassName() = delete; \
		Z_NO_COPY_MOVE(ClassName)

// ћакрос позвол€ющий избежать ручного ввода std::source_location::current()
#define UNEXPECTED(code, fmt, ...) Unexpected(code, std::source_location::current(), fmt, ##__VA_ARGS__)