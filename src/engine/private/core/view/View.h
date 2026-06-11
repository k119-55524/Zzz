#pragma once

#include "../../platforms/Platform.h"
#include "../../platforms/input/Input.h"
#include "../../platforms/window/Window.h"

namespace zzz::engine
{
	class View final
	{
		Z_NO_MOVE(View);

	public:
		View() = delete;
		View(std::shared_ptr<Platform> platform, std::function<void(View&)> onWindowClose);
		~View() = default;

	private:
		void Initialize();

		std::shared_ptr<Platform> m_Platform;
		std::shared_ptr<Window> m_Window;
		std::shared_ptr<Input>  m_Input;

		std::function<void(View&)> OnWindowClose;
		void HandleWindowClose() { OnWindowClose(*this); }
	};
}
