#pragma once

#include <functional>
#include <bitset>
#include "InputHelpers.h"
#include <engine/header.h>

#if Z_ANDROID
#include <android/input.h>
#endif

namespace zzz::engine
{
	class IWindow;

#pragma region Type Aliases
#if Z_WINDOWS
	struct WinMsg
	{
		HWND	hWnd;
		UINT	uMsg;
		WPARAM	wParam;
		LPARAM	lParam;
	};
	using NativeMsg = WinMsg;
#elif Z_ANDROID
	struct AndroidMsg
	{
		void* motionEvent;
		void* keyEvent;
	};
	using NativeMsg = AndroidMsg;
#elif Z_LINUX
	struct LinuxMsg
	{
		void* msg;
	};
	using NativeMsg = LinuxMsg;
#elif Z_MACOS
	struct MacOSMsg
	{
		void* msg;
	};
	using NativeMsg = MacOSMsg;
#elif Z_IOS
	struct iOSMsg
	{
		void* msg;
	};
	using NativeMsg = iOSMsg;
#else
#error ">>>>> InputBase: Unsupported platform."
#endif
#pragma endregion

	class InputBase
	{
	public:
#pragma region Mouse events
		Event<bool> OnMouseEnter;

		Event<zI32, zI32> OnMouseDelta;
		Event<zI32> OnMouseWheelVertical;
		Event<zI32> OnMouseWheelHorizontal;

		Event<MouseButton, bool> OnMouseButton;
		Event<bool> OnMouseLeftButton;
		Event<bool> OnMouseRightButton;
		Event<bool> OnMouseMiddleButton;
		Event<bool> OnMouseButton4;
		Event<bool> OnMouseButton5;

		[[nodiscard]] bool IsMouseButtonDown(MouseButton button) const { return (m_MouseButtonsMask & (1 << static_cast<zU32>(button))) != 0; }
#pragma endregion

#pragma region Keyboard events
		Event<KeyCode, KeyState> OnKeyStateChanged;
		Event<KeyCode> OnKeyDown;
		Event<KeyCode> OnKeyUp;

		[[nodiscard]] bool IsKeyDown(KeyCode key) const
		{
			if (key == KeyCode::Unknown)
			{
				DOut("Передан KeyCode::Unknown");
				return false;
			}

			zU32 index = static_cast<zU32>(key);

			if (index < static_cast<zU32>(KeyCode::Count))
				return m_KeyStates[index];

			DOut("Некорректное значение кода клавиши ({})", static_cast<zI32>(key));
			return false;
		}

		void ResetState()
		{
			// Сброс мыши
			for (zU32 i = 0; i < 5; ++i)
			{
				zU32 bit = 1 << i;
				if (m_MouseButtonsMask & bit)
				{
					m_MouseButtonsMask &= ~bit;
					MouseButton btn = static_cast<MouseButton>(i);
					OnMouseButton(btn, false);
					switch (btn)
					{
					case MouseButton::Left:    OnMouseLeftButton(false);   break;
					case MouseButton::Right:   OnMouseRightButton(false);  break;
					case MouseButton::Middle:  OnMouseMiddleButton(false); break;
					case MouseButton::Button4: OnMouseButton4(false);      break;
					case MouseButton::Button5: OnMouseButton5(false);      break;
					default: break;
					}
				}
			}

			// Сброс клавиатуры
			for (size_t i = 0; i < static_cast<size_t>(KeyCode::Count); ++i)
			{
				if (m_KeyStates[i])
				{
					m_KeyStates[i] = false;
					KeyCode key = static_cast<KeyCode>(i);
					OnKeyStateChanged(key, KeyState::Up);
					OnKeyUp(key);
				}
			}
		}

	protected:
		InputBase();
		~InputBase() = default;

		void UpdateMouseButtonState(MouseButton button, bool pressed)
		{
			zU32 bit = 1 << static_cast<zU32>(button);
			if (pressed)
				m_MouseButtonsMask |= bit;
			else
				m_MouseButtonsMask &= ~bit;

			OnMouseButton(button, pressed);
			switch (button)
			{
			case MouseButton::Left:    OnMouseLeftButton(pressed);   break;
			case MouseButton::Right:   OnMouseRightButton(pressed);  break;
			case MouseButton::Middle:  OnMouseMiddleButton(pressed); break;
			case MouseButton::Button4: OnMouseButton4(pressed);      break;
			case MouseButton::Button5: OnMouseButton5(pressed);      break;
			default: break;
			}
		}

		void UpdateKeyState(KeyCode key, bool pressed)
		{
			if (key == KeyCode::Unknown)
				return;

			zU32 index = static_cast<zU32>(key);
			if (index < static_cast<zU32>(KeyCode::Count))
			{
				if (m_KeyStates[index] != pressed)
				{
					m_KeyStates[index] = pressed;
					OnKeyStateChanged(key, pressed ? KeyState::Down : KeyState::Up);
					
					if (pressed)
						OnKeyDown(key);
					else
						OnKeyUp(key);
				}
			}
		}

		bool m_IsMouseInside;

	private:
		zU32 m_MouseButtonsMask = 0;
		std::bitset<static_cast<size_t>(KeyCode::Count)> m_KeyStates;
	};
}

