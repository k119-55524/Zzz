
export module InputFactory;

import Ensure;
import IMouse;
import IAppWin;
import IKeyboard;

#if defined(ZPLATFORM_MSWINDOWS)
import Mouse_MSWin;
import AppWin_MSWin;
import Keyboard_MSWin;
#else
#error >>>>> This branch requires implementation for the current platform
#endif

using namespace zzz::core;

namespace zzz::input
{
	export class InputFactory
	{
	public:
		explicit InputFactory() = default;
		~InputFactory() = default;

		[[nodiscard]] std::shared_ptr<IMouse> CreateInterfaceMouse(const std::shared_ptr<IAppWin> appWindow) noexcept;
		[[nodiscard]] std::shared_ptr<IKeyboard> CreateInterfaceKeyboard(const std::shared_ptr<IAppWin> appWindow) noexcept;
	};

	std::shared_ptr<IMouse> InputFactory::CreateInterfaceMouse(const std::shared_ptr<IAppWin> appWindow) noexcept
	{
#if defined(ZPLATFORM_MSWINDOWS)
		auto win = std::dynamic_pointer_cast<AppWin_MSWin>(appWindow);
		ensure(win, "Application window is not of type IAppWin_MSWin.");

		return std::make_shared<Mouse_MSWin>(win);
#else
#error >>>>> This branch requires implementation for the current platform
#endif
	}

	std::shared_ptr<IKeyboard> InputFactory::CreateInterfaceKeyboard(const std::shared_ptr<IAppWin> appWindow) noexcept
	{
#if defined(ZPLATFORM_MSWINDOWS)
		auto win = std::dynamic_pointer_cast<AppWin_MSWin>(appWindow);
		ensure(win, "Application window is not of type IAppWin_MSWin.");

		return std::make_shared<Keyboard_MSWin>(win);
#else
#error >>>>> This branch requires implementation for the current platform
#endif
	}
}