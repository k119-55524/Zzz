#pragma once

#include "InputBase.h"

namespace zzz::engine
{
	class InputEditor final : public InputBase
	{
	public:
		InputEditor();
		~InputEditor();

		[[nodiscard]] std::expected<void, std::string> Initialize();

		// Публичные методы для WPF редактора (для ручного вброса событий в будущем)
		void InjectKeyDown(int key);
		void InjectKeyUp(int key);
		void InjectMouseMove(int x, int y);
		void InjectMouseButtonDown(int button);
		void InjectMouseButtonUp(int button);
	};
}
