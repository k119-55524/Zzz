
export module Input;

export import KeyCode;

import IMouse;
import IAppWin;
import IKeyboard;
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
			ensure(m_AppWin, "Application window cannot be null.");
			Initialize();
		}
		~Input() = default;

		inline KeyState GetKeyState(KeyCode key) const { return m_Keyboard->GetKeyState(key); };

	private:
		void Initialize();

		void OnWinFocus(bool focus);
		void OnWinActivate(bool activate);

		std::shared_ptr<IAppWin> m_AppWin;

		InputFactory m_Factory;
		std::shared_ptr<IMouse> m_Mouse;
		std::shared_ptr<IKeyboard> m_Keyboard;
	};

	void Input::Initialize()
	{
		m_AppWin->OnFocus += std::bind(&Input::OnWinFocus, this, std::placeholders::_1);
		m_AppWin->OnActivate += std::bind(&Input::OnWinActivate, this, std::placeholders::_1);

		m_Mouse = m_Factory.CreateInterfaceMouse(m_AppWin);
		m_Keyboard = m_Factory.CreateInterfaceKeyboard(m_AppWin);
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