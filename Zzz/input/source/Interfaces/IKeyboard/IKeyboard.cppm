
export module IKeyboard;

import Event;
import KeyCode;

using namespace zzz;
using namespace zzz::core;

namespace zzz::input
{
	class KeyButtonWatcher
	{
	public:
		KeyButtonWatcher() :
			m_KeyState{KeyState::Up}
		{
		}
		~KeyButtonWatcher() = default;

		inline void SetState(KeyState state) noexcept
		{
			if (m_KeyState != state)
			{
				m_KeyState = state;

				if (state == KeyState::Up)
					OnUp();
				else
					OnDown();
			}
		}
		inline KeyState GetState() const noexcept { return m_KeyState; }

		Event<> OnUp;
		Event<> OnDown;

	private:
		KeyState m_KeyState;
	};

	/// <summary>
	/// Интерфейс для работы с клавиатурой
	/// </summary>
	export class IKeyboard
	{
	public:
		explicit IKeyboard()
		{
		};
		virtual ~IKeyboard() = 0;

		inline KeyState GetKeyState(KeyCode key) const { return Keys[static_cast<int>(key)].GetState(); };
		inline KeyButtonWatcher& GetButtonWatcher(KeyCode key) noexcept { return Keys[static_cast<int>(key)]; }

	protected:
		void OnKeyStateChanged(KeyCode key, KeyState state);
		void OnFocus(bool focus);

		KeyButtonWatcher Keys[static_cast<int>(KeyCode::Count)];
	};

	IKeyboard::~IKeyboard() {};

	void IKeyboard::OnKeyStateChanged(KeyCode key, KeyState state)
	{
		//DebugOutput(std::format(L">>>>> [IKeyboard::OnKeyStateChanged()]. Key: {}, state: {}.", ToString(key), ToString(state)));
 		Keys[static_cast<int>(key)].SetState(state);
	}

	void IKeyboard::OnFocus(bool focus)
	{
		if (!focus)
		{
			for (auto& key : Keys)
				key.SetState(KeyState::Up);
		}
	}
}