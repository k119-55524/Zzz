#pragma once

// Макрос для запрета копирования и перемещения класса
#define Z_NO_COPY_MOVE(ClassName) \
	private: \
		ClassName(const ClassName&) = delete; \
		ClassName(ClassName&&) = delete; \
		ClassName& operator=(const ClassName&) = delete; \
		ClassName& operator=(ClassName&&) = delete;

	// Макрос для запрета создания с конструктором без параметров, копирования и перемещения класса
#define Z_NO_CREATE_COPY(ClassName) \
	private: \
		ClassName() = delete; \
		Z_NO_COPY_MOVE(ClassName)