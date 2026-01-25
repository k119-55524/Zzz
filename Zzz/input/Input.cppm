
export module Input;

import IMouse;
import IAppWin;
import InputFactory;

using namespace zzz::core;

namespace zzz::input
{
	export class Input final
	{
		public:
			explicit Input(const std::shared_ptr<IAppWin> appWindow)
			{
				ensure(appWindow, ">>>>> [Input::Input()]. Application window cannot be null.");
				Initialize(appWindow);
			}
			~Input() = default;

	private:
		void Initialize(const std::shared_ptr<IAppWin> appWindow);

		InputFactory factory;
		std::shared_ptr<IMouse> m_Mouse;
	};

	void Input::Initialize(const std::shared_ptr<IAppWin> appWindow)
	{
		m_Mouse = factory.CreateInterfaceMouse(appWindow);
	}
}