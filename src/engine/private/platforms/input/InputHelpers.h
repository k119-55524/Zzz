#pragma once

#include <array>

namespace zzz::engine
{
	enum class MouseButton : zU8
	{
		Left = 0,
		Right = 1,
		Middle = 2,
		Button3 = 2, // Синоним для Middle
		Button4 = 3,
		Button5 = 4,

		Count = 5
	};

	enum class KeyState : zU32
	{
		Up = 0,
		Down = 1
	};

	enum class KeyCode : zI32
	{
		Unknown = 0,

		// -----------------------------
		// Буквы алфавита
		// -----------------------------
		A, B, C, D, E, F, G,
		H, I, J, K, L, M, N,
		O, P, Q, R, S, T,
		U, V, W, X, Y, Z,

		// -----------------------------
		// Ряд цифр (не Numpad)
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
		// Функциональные клавиши
		// -----------------------------
		F1, F2, F3, F4,
		F5, F6, F7, F8,
		F9, F10, F11, F12,
		F13, F14, F15,
		F16, F17, F18,
		F19, F20, F21,
		F22, F23, F24,

		// -----------------------------
		// Управляющие клавиши
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
		// Клавиши стрелок
		// -----------------------------
		ArrowUp,
		ArrowDown,
		ArrowLeft,
		ArrowRight,

		// -----------------------------
		// Клавиши-модификаторы
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
		// Клавиши-фиксаторы
		// -----------------------------
		CapsLock,
		NumLock,
		ScrollLock,

		// -----------------------------
		// Цифровая клавиатура (Numpad)
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
		// Символы (логическая US-раскладка)
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
		// Системные / особые
		// -----------------------------
		PrintScreen,
		Pause,
		Menu,

		// -----------------------------
		// Медиа-клавиши (опционально)
		// -----------------------------
		VolumeUp,
		VolumeDown,
		VolumeMute,

		MediaPlayPause,
		MediaStop,
		MediaNext,
		MediaPrevious,

		// -----------------------------
		// Псевдо-клавиши мыши (опционально)
		// -----------------------------
		MouseLeft,
		MouseRight,
		MouseMiddle,
		MouseButton4,
		MouseButton5,

		// -----------------------------
		// Технический элемент-ограничитель (для подсчёта количества)
		// -----------------------------
		Count
	};

	// ----------------------------------------------------------------
	// Константная таблица MS Windows VirtualKeyboard -> KeyCode
	// ----------------------------------------------------------------
	constexpr std::array<KeyCode, 512> MSWinVirtualKeyMap = []
	{
		std::array<KeyCode, 512> arr{};

		// Инициализация всех Unknown
		for (auto& k : arr) k = KeyCode::Unknown;

		// -----------------------------
		// Буквы A-Z
		// -----------------------------
		for (int i = 0; i < 26; ++i)
			arr['A' + i] = static_cast<KeyCode>(static_cast<zI32>(KeyCode::A) + i);

		// -----------------------------
		// Цифры 0-9 (верхний ряд)
		// -----------------------------
		for (int i = 0; i < 10; ++i)
			arr['0' + i] = static_cast<KeyCode>(static_cast<zI32>(KeyCode::Digit0) + i);

		// -----------------------------
		// Функциональные клавиши
		// -----------------------------
		arr[0x70] = KeyCode::F1;  arr[0x71] = KeyCode::F2;
		arr[0x72] = KeyCode::F3;  arr[0x73] = KeyCode::F4;
		arr[0x74] = KeyCode::F5;  arr[0x75] = KeyCode::F6;
		arr[0x76] = KeyCode::F7;  arr[0x77] = KeyCode::F8;
		arr[0x78] = KeyCode::F9;  arr[0x79] = KeyCode::F10;
		arr[0x7A] = KeyCode::F11; arr[0x7B] = KeyCode::F12;
		arr[0x7C] = KeyCode::F13; arr[0x7D] = KeyCode::F14;
		arr[0x7E] = KeyCode::F15; arr[0x7F] = KeyCode::F16;
		arr[0x80] = KeyCode::F17; arr[0x81] = KeyCode::F18;
		arr[0x82] = KeyCode::F19; arr[0x83] = KeyCode::F20;
		arr[0x84] = KeyCode::F21; arr[0x85] = KeyCode::F22;
		arr[0x86] = KeyCode::F23; arr[0x87] = KeyCode::F24;

		// -----------------------------
		// Управляющие клавиши
		// -----------------------------
		arr[0x1B] = KeyCode::Escape;
		arr[0x0D] = KeyCode::Enter;
		arr[0x09] = KeyCode::Tab;
		arr[0x08] = KeyCode::Backspace;
		arr[0x20] = KeyCode::Space;
		arr[0x2D] = KeyCode::Insert;
		arr[0x2E] = KeyCode::Delete;
		arr[0x24] = KeyCode::Home;
		arr[0x23] = KeyCode::End;
		arr[0x21] = KeyCode::PageUp;
		arr[0x22] = KeyCode::PageDown;

		// -----------------------------
		// Клавиши стрелок
		// -----------------------------
		arr[0x26] = KeyCode::ArrowUp;
		arr[0x28] = KeyCode::ArrowDown;
		arr[0x25] = KeyCode::ArrowLeft;
		arr[0x27] = KeyCode::ArrowRight;

		// -----------------------------
		// Модификаторы
		// -----------------------------
		arr[0xA0] = KeyCode::LeftShift;  arr[0xA1] = KeyCode::RightShift;
		arr[0xA2] = KeyCode::LeftCtrl;   arr[0xA3] = KeyCode::RightCtrl;
		arr[0xA4] = KeyCode::LeftAlt;    arr[0xA5] = KeyCode::RightAlt;

		// -----------------------------
		// Фиксаторы
		// -----------------------------
		arr[0x14] = KeyCode::CapsLock;
		arr[0x90] = KeyCode::NumLock;
		arr[0x91] = KeyCode::ScrollLock;

		// -----------------------------
		// Цифровая клавиатура (Numpad)
		// -----------------------------
		arr[0x60] = KeyCode::Numpad0; arr[0x61] = KeyCode::Numpad1; arr[0x62] = KeyCode::Numpad2;
		arr[0x63] = KeyCode::Numpad3; arr[0x64] = KeyCode::Numpad4; arr[0x65] = KeyCode::Numpad5;
		arr[0x66] = KeyCode::Numpad6; arr[0x67] = KeyCode::Numpad7; arr[0x68] = KeyCode::Numpad8;
		arr[0x69] = KeyCode::Numpad9;
		arr[0x6B] = KeyCode::NumpadAdd; arr[0x6D] = KeyCode::NumpadSubtract;
		arr[0x6A] = KeyCode::NumpadMultiply; arr[0x6F] = KeyCode::NumpadDivide;
		arr[0x6E] = KeyCode::NumpadDecimal;
		// NumPad Enter – расширенный Enter (E0). Обрабатывается в TranslateMSWinKey.

		// -----------------------------
		// Символы
		// -----------------------------
		arr[0xBD] = KeyCode::Minus;    arr[0xBB] = KeyCode::Equals;
		arr[0xDB] = KeyCode::LeftBracket; arr[0xDD] = KeyCode::RightBracket;
		arr[0xDC] = KeyCode::Backslash;  arr[0xBA] = KeyCode::Semicolon;
		arr[0xDE] = KeyCode::Apostrophe; arr[0xBC] = KeyCode::Comma;
		arr[0xBE] = KeyCode::Period;     arr[0xBF] = KeyCode::Slash;
		arr[0xC0] = KeyCode::Grave;

		// -----------------------------
		// Системные / особые
		// -----------------------------
		arr[0x2C] = KeyCode::PrintScreen;
		arr[0x13] = KeyCode::Pause;
		arr[0x5D] = KeyCode::Menu;

		return arr;
	}();
}