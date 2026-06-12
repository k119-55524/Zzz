#pragma once

#include <functional>
#include "InputHelpers.h"
#include "../../core/templates/Event.h"

#if Z_WINDOWS
#include "../../../headers/MSWin.h"
#elif Z_ANDROID
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
#error ">>>>> Unsupported platform"
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
#pragma endregion

#pragma region Rteboard events
		Event<KeyCode, KeyState> OnKeyStateChanged;
#pragma endregion

		// ---------------------------------------------------------------------
		// Состояние мыши
		// ---------------------------------------------------------------------
		[[nodiscard]] bool IsMouseButtonDown(MouseButton button) const
		{
			return (m_MouseButtonsMask & (1 << static_cast<zU32>(button))) != 0;
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

		bool IsMouseInside;

	private:
		zU32 m_MouseButtonsMask = 0;
	};
}
