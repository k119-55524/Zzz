
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
			explicit Input(const std::shared_ptr<IAppWin> appWin) :
				m_AppWin{ appWin }
			{
				ensure(m_AppWin, ">>>>> [Input::Input()]. Application window cannot be null.");
				Initialize();
			}
			~Input() = default;

	private:
		void Initialize();

		void OnWinFocus(bool focus);
		void OnWinActivate(bool activate);

		std::shared_ptr<IAppWin> m_AppWin;

		InputFactory factory;
		std::shared_ptr<IMouse> m_Mouse;
	};

	void Input::Initialize()
	{
		m_AppWin->OnFocus += std::bind(&Input::OnWinFocus, this, std::placeholders::_1);
		m_AppWin->OnActivate += std::bind(&Input::OnWinActivate, this, std::placeholders::_1);

		m_Mouse = factory.CreateInterfaceMouse(m_AppWin);
	}

	void Input::OnWinFocus(bool focus)
	{
		//DebugOutput(std::format(L">>>>> [Input::OnWinFocus()]. Window focus changed: {}", focus ? L"true" : L"false"));
	}

	void Input::OnWinActivate(bool activate)
	{
		//DebugOutput(std::format(L">>>>> [Input::OnWinActivate()]. Window activation changed: {}", activate ? L"true" : L"false"));
	}
}