#pragma once

namespace zzz::engine
{
	// Битовая маска кнопок мыши
	enum class MouseButtonMask : zU32
	{
		None = 0,
		Left = 1 << 0,
		Right = 1 << 1,
		Middle = 1 << 2,
		Button4 = 1 << 3,
		Button5 = 1 << 4
	};

	inline MouseButtonMask operator|(MouseButtonMask a, MouseButtonMask b) { return static_cast<MouseButtonMask>(static_cast<zU8>(a) | static_cast<zU8>(b)); }
	inline MouseButtonMask& operator|=(MouseButtonMask& a, MouseButtonMask b) { a = a | b; return a; }
	inline MouseButtonMask operator&(MouseButtonMask a, MouseButtonMask b) { return static_cast<MouseButtonMask>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b)); }

	export enum class KeyState : zU32
	{
		Up = 0,
		Down = 1
	};

	export enum class KeyCode : zI32
	{
		Unknown = 0,

		// -----------------------------
		// Alphabet
		// -----------------------------
		A, B, C, D, E, F, G,
		H, I, J, K, L, M, N,
		O, P, Q, R, S, T,
		U, V, W, X, Y, Z,

		// -----------------------------
		// Number row (not numpad)
		// -----------------------------
		Digit0,
		Digit1,
		Digit2,
		Digit3,
		Digit4,
		Digit5,
		Digit6,
		Digit7,
		Digit8,
		Digit9,

		// -----------------------------
		// Function keys
		// -----------------------------
		F1, F2, F3, F4,
		F5, F6, F7, F8,
		F9, F10, F11, F12,
		F13, F14, F15,
		F16, F17, F18,
		F19, F20, F21,
		F22, F23, F24,

		// -----------------------------
		// Control keys
		// -----------------------------
		Escape,
		Enter,
		Tab,
		Backspace,
		Space,

		Insert,
		Delete,
		Home,
		End,
		PageUp,
		PageDown,

		// -----------------------------
		// Arrow keys
		// -----------------------------
		ArrowUp,
		ArrowDown,
		ArrowLeft,
		ArrowRight,

		// -----------------------------
		// Modifier keys
		// -----------------------------
		LeftShift,
		RightShift,
		LeftCtrl,
		RightCtrl,
		LeftAlt,
		RightAlt,
		LeftMeta,     // Windows / Command
		RightMeta,

		// -----------------------------
		// Lock keys
		// -----------------------------
		CapsLock,
		NumLock,
		ScrollLock,

		// -----------------------------
		// Numpad
		// -----------------------------
		Numpad0,
		Numpad1,
		Numpad2,
		Numpad3,
		Numpad4,
		Numpad5,
		Numpad6,
		Numpad7,
		Numpad8,
		Numpad9,

		NumpadAdd,
		NumpadSubtract,
		NumpadMultiply,
		NumpadDivide,
		NumpadDecimal,
		NumpadEnter,

		// -----------------------------
		// Symbols (US layout logical)
		// -----------------------------
		Minus,        // -
		Equals,       // =
		LeftBracket,  // [
		RightBracket, // ]
		Backslash,    // '\'
		Semicolon,    // ;
		Apostrophe,   // '
		Comma,        // ,
		Period,       // .
		Slash,        // /
		Grave,        // `

		// -----------------------------
		// System / special
		// -----------------------------
		PrintScreen,
		Pause,
		Menu,

		// -----------------------------
		// Media keys (optional)
		// -----------------------------
		VolumeUp,
		VolumeDown,
		VolumeMute,

		MediaPlayPause,
		MediaStop,
		MediaNext,
		MediaPrevious,

		// -----------------------------
		// Mouse pseudo-keys (optional)
		// -----------------------------
		MouseLeft,
		MouseRight,
		MouseMiddle,
		MouseButton4,
		MouseButton5,

		// -----------------------------
		// Sentinel
		// -----------------------------
		Count
	};
}