
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
		explicit IKeyboard() = default;
		virtual ~IKeyboard() = 0;

		void OnKeyStateChanged(KeyCode key, KeyState state);
	};

	IKeyboard::~IKeyboard() {};

	void IKeyboard::OnKeyStateChanged(KeyCode key, KeyState state)
	{
		//DebugOutput(std::format(L">>>>> [IKeyboard::OnKeyStateChanged()]. Key: {}, state: {}.", ToString(key), ToString(state)));
	}
}