#pragma once

#include "../../../header.h"

#include "../../platforms/Platform.h"
#include "../../platforms/window/Window.h"

#include "../../platforms/input/Input.h"

namespace zzz::engine
{
	class NativeView final
	{
		Z_NO_MOVE(NativeView);

	public:
		NativeView() = delete;
		NativeView(std::shared_ptr<Platform> platform);
		~NativeView() = default;

		inline std::shared_ptr<Window> GetWindow() const noexcept { return m_Window; }
		inline std::shared_ptr<Input>  GetInput()  const noexcept { return m_Input; }

	private:
		void Initialize();

		std::shared_ptr<Platform> m_Platform;
		std::shared_ptr<Window> m_Window;
		std::shared_ptr<Input>  m_Input;
	};
}
