#pragma once

#include "../../../header.h"

#include "../Platform.h"
#include "../../platforms/native_view/window/IWindow.h"

#include "../../inputs/IInput.h"

namespace zzz::engine
{
	class NativeView final
	{
		Z_NO_MOVE(NativeView);

	public:
		NativeView() = delete;
		NativeView(std::shared_ptr<Platform> platform);
		~NativeView() = default;

		inline std::shared_ptr<IWindow> GetWindow() const noexcept { return m_Window; }
		inline std::shared_ptr<IInput>  GetInput()  const noexcept { return m_Input; }

	private:
		void Initialize();

		std::shared_ptr<Platform> m_Platform;
		std::shared_ptr<IWindow> m_Window;
		std::shared_ptr<IInput>  m_Input;
	};
}
