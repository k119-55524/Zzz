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
		View(std::shared_ptr<Platform> platform);
		~View() = default;

		inline std::shared_ptr<Window> GetWindow() const noexcept { return m_Window; }

	private:
		void Initialize();

		std::shared_ptr<Platform> m_Platform;
		std::shared_ptr<Window> m_Window;
		std::shared_ptr<Input>  m_Input;
	};
}
