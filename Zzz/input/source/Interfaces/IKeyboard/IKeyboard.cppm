
export module IKeyboard;

import KeyCode;

using namespace zzz::core;

namespace zzz::input
{
	/// <summary>
	/// Интерфейс для работы с клавиатурой
	/// </summary>
	export class IKeyboard
	{
	public:
		explicit IKeyboard()
		{
			std::fill(Keys, Keys + static_cast<int>(KeyCode::Count), KeyState::Up);
		};
		virtual ~IKeyboard() = 0;

		void OnKeyStateChanged(KeyCode key, KeyState state);
		inline KeyState GetKeyState(KeyCode key) const { return Keys[static_cast<int>(key)]; };

	protected:
		KeyState Keys[static_cast<int>(KeyCode::Count)] = { KeyState::Up };
	};

	IKeyboard::~IKeyboard() {};

	void IKeyboard::OnKeyStateChanged(KeyCode key, KeyState state)
	{
		//DebugOutput(std::format(L">>>>> [IKeyboard::OnKeyStateChanged()]. Key: {}, state: {}.", ToString(key), ToString(state)));
 		Keys[static_cast<int>(key)] = state;
	}
}