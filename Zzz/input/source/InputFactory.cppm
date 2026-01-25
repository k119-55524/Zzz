
export module InputFactory;

import IMouse;
import IAppWin;
import MouseNull;

#if defined(ZPLATFORM_MSWINDOWS)
import MouseWin;
import AppWin_MSWin;
#else
#error ">>>>> [Compile error]. This branch requires implementation for the current platform"
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
	};

	std::shared_ptr<IMouse> InputFactory::CreateInterfaceMouse(const std::shared_ptr<IAppWin> appWindow) noexcept
	{
#if defined(ZPLATFORM_MSWINDOWS)
		auto win = std::dynamic_pointer_cast<AppWin_MSWin>(appWindow);
		ensure(win, ">>>>> [InputFactory::CreateInterfaceMouse(...)]. Application window is not of type IAppWin_MSWin.");

		return std::make_shared<MouseWin>(win);
#else
		return std::make_shared<MouseNull>();
#endif
	}
}