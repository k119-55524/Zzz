#pragma once

#include <string>
#include <expected>
#include <foundation.h>

namespace zzz::engine
{
	class IWindow;

	class IInput
	{
		Z_NO_COPY_MOVE(IInput);

	public:
		IInput() = default;
		virtual ~IInput() = default;

		[[nodiscard]] virtual std::expected<void, std::string> Initialize() = 0;

		// Метод для обработки системных сообщений (Windows/Linux)
		// На Android этот метод может принимать другие параметры или быть перегружен
		virtual bool ProcessMessage(void* nativeMsg) = 0;
	};
}
