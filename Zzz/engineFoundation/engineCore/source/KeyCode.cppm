
export module KeyCode;

namespace zzz::core
{
	export enum class KeyState : zI32
	{
		Up		= 0,
		Down	= 1
	};

	export constexpr const wchar_t* ToString(KeyState state)
	{
		switch (state)
		{
		case KeyState::Down: return L"Down";
		case KeyState::Up:   return L"Up";
		}
		return L"Unknown";
	}

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

	export constexpr const wchar_t* ToString(KeyCode key)
	{
		switch (key)
		{
			// -----------------------------
			// Alphabet
			// -----------------------------
		case KeyCode::A: return L"A";
		case KeyCode::B: return L"B";
		case KeyCode::C: return L"C";
		case KeyCode::D: return L"D";
		case KeyCode::E: return L"E";
		case KeyCode::F: return L"F";
		case KeyCode::G: return L"G";
		case KeyCode::H: return L"H";
		case KeyCode::I: return L"I";
		case KeyCode::J: return L"J";
		case KeyCode::K: return L"K";
		case KeyCode::L: return L"L";
		case KeyCode::M: return L"M";
		case KeyCode::N: return L"N";
		case KeyCode::O: return L"O";
		case KeyCode::P: return L"P";
		case KeyCode::Q: return L"Q";
		case KeyCode::R: return L"R";
		case KeyCode::S: return L"S";
		case KeyCode::T: return L"T";
		case KeyCode::U: return L"U";
		case KeyCode::V: return L"V";
		case KeyCode::W: return L"W";
		case KeyCode::X: return L"X";
		case KeyCode::Y: return L"Y";
		case KeyCode::Z: return L"Z";

			// -----------------------------
			// Digits
			// -----------------------------
		case KeyCode::Digit0: return L"0";
		case KeyCode::Digit1: return L"1";
		case KeyCode::Digit2: return L"2";
		case KeyCode::Digit3: return L"3";
		case KeyCode::Digit4: return L"4";
		case KeyCode::Digit5: return L"5";
		case KeyCode::Digit6: return L"6";
		case KeyCode::Digit7: return L"7";
		case KeyCode::Digit8: return L"8";
		case KeyCode::Digit9: return L"9";

			// -----------------------------
			// Function keys
			// -----------------------------
		case KeyCode::F1:  return L"F1";
		case KeyCode::F2:  return L"F2";
		case KeyCode::F3:  return L"F3";
		case KeyCode::F4:  return L"F4";
		case KeyCode::F5:  return L"F5";
		case KeyCode::F6:  return L"F6";
		case KeyCode::F7:  return L"F7";
		case KeyCode::F8:  return L"F8";
		case KeyCode::F9:  return L"F9";
		case KeyCode::F10: return L"F10";
		case KeyCode::F11: return L"F11";
		case KeyCode::F12: return L"F12";
		case KeyCode::F13: return L"F13";
		case KeyCode::F14: return L"F14";
		case KeyCode::F15: return L"F15";
		case KeyCode::F16: return L"F16";
		case KeyCode::F17: return L"F17";
		case KeyCode::F18: return L"F18";
		case KeyCode::F19: return L"F19";
		case KeyCode::F20: return L"F20";
		case KeyCode::F21: return L"F21";
		case KeyCode::F22: return L"F22";
		case KeyCode::F23: return L"F23";
		case KeyCode::F24: return L"F24";

			// -----------------------------
			// Control
			// -----------------------------
		case KeyCode::Escape:   return L"Escape";
		case KeyCode::Enter:    return L"Enter";
		case KeyCode::Tab:      return L"Tab";
		case KeyCode::Backspace:return L"Backspace";
		case KeyCode::Space:    return L"Space";

		case KeyCode::Insert:   return L"Insert";
		case KeyCode::Delete:   return L"Delete";
		case KeyCode::Home:     return L"Home";
		case KeyCode::End:      return L"End";
		case KeyCode::PageUp:   return L"PageUp";
		case KeyCode::PageDown: return L"PageDown";

			// -----------------------------
			// Arrows
			// -----------------------------
		case KeyCode::ArrowUp:    return L"ArrowUp";
		case KeyCode::ArrowDown:  return L"ArrowDown";
		case KeyCode::ArrowLeft:  return L"ArrowLeft";
		case KeyCode::ArrowRight: return L"ArrowRight";

			// -----------------------------
			// Modifiers
			// -----------------------------
		case KeyCode::LeftShift:  return L"LeftShift";
		case KeyCode::RightShift: return L"RightShift";
		case KeyCode::LeftCtrl:   return L"LeftCtrl";
		case KeyCode::RightCtrl:  return L"RightCtrl";
		case KeyCode::LeftAlt:    return L"LeftAlt";
		case KeyCode::RightAlt:   return L"RightAlt";
		case KeyCode::LeftMeta:   return L"LeftMeta";
		case KeyCode::RightMeta:  return L"RightMeta";

			// -----------------------------
			// Locks
			// -----------------------------
		case KeyCode::CapsLock:   return L"CapsLock";
		case KeyCode::NumLock:    return L"NumLock";
		case KeyCode::ScrollLock: return L"ScrollLock";

			// -----------------------------
			// Numpad
			// -----------------------------
		case KeyCode::Numpad0: return L"Numpad0";
		case KeyCode::Numpad1: return L"Numpad1";
		case KeyCode::Numpad2: return L"Numpad2";
		case KeyCode::Numpad3: return L"Numpad3";
		case KeyCode::Numpad4: return L"Numpad4";
		case KeyCode::Numpad5: return L"Numpad5";
		case KeyCode::Numpad6: return L"Numpad6";
		case KeyCode::Numpad7: return L"Numpad7";
		case KeyCode::Numpad8: return L"Numpad8";
		case KeyCode::Numpad9: return L"Numpad9";

		case KeyCode::NumpadAdd:      return L"NumpadAdd";
		case KeyCode::NumpadSubtract: return L"NumpadSubtract";
		case KeyCode::NumpadMultiply: return L"NumpadMultiply";
		case KeyCode::NumpadDivide:   return L"NumpadDivide";
		case KeyCode::NumpadDecimal:  return L"NumpadDecimal";
		case KeyCode::NumpadEnter:    return L"NumpadEnter";

			// -----------------------------
			// Symbols
			// -----------------------------
		case KeyCode::Minus:        return L"-";
		case KeyCode::Equals:       return L"=";
		case KeyCode::LeftBracket:  return L"[";
		case KeyCode::RightBracket: return L"]";
		case KeyCode::Backslash:    return L"\\";
		case KeyCode::Semicolon:    return L";";
		case KeyCode::Apostrophe:   return L"'";
		case KeyCode::Comma:        return L",";
		case KeyCode::Period:       return L".";
		case KeyCode::Slash:        return L"/";
		case KeyCode::Grave:        return L"`";

			// -----------------------------
			// System
			// -----------------------------
		case KeyCode::PrintScreen: return L"PrintScreen";
		case KeyCode::Pause:       return L"Pause";
		case KeyCode::Menu:        return L"Menu";

			// -----------------------------
			// Media
			// -----------------------------
		case KeyCode::VolumeUp:       return L"VolumeUp";
		case KeyCode::VolumeDown:     return L"VolumeDown";
		case KeyCode::VolumeMute:     return L"VolumeMute";
		case KeyCode::MediaPlayPause: return L"MediaPlayPause";
		case KeyCode::MediaStop:      return L"MediaStop";
		case KeyCode::MediaNext:      return L"MediaNext";
		case KeyCode::MediaPrevious:  return L"MediaPrevious";

			// -----------------------------
			// Mouse
			// -----------------------------
		case KeyCode::MouseLeft:    return L"MouseLeft";
		case KeyCode::MouseRight:   return L"MouseRight";
		case KeyCode::MouseMiddle:  return L"MouseMiddle";
		case KeyCode::MouseButton4: return L"MouseButton4";
		case KeyCode::MouseButton5: return L"MouseButton5";

		case KeyCode::Unknown:
		case KeyCode::Count:
		default:
			return L"Unknown";
		}
	}
}

#if defined(ZPLATFORM_MSWINDOWS)
namespace zzz::core
{
	// ----------------------------------------------------------------
	// Константная таблица MS Windows VirtualKeyboard -> KeyCode
	// ----------------------------------------------------------------
	constexpr std::array<KeyCode, 512> MSWinVirtualKeyMap = [] {
		std::array<KeyCode, 512> arr{};

		// Инициализация всех Unknown
		for (auto& k : arr) k = KeyCode::Unknown;

		// -----------------------------
		// Alphabet A-Z
		// -----------------------------
		for (int i = 0; i < 26; ++i)
			arr['A' + i] = static_cast<KeyCode>(static_cast<zI32>(KeyCode::A) + i);

		// -----------------------------
		// Digits 0-9 (row)
		// -----------------------------
		for (int i = 0; i < 10; ++i)
			arr['0' + i] = static_cast<KeyCode>(static_cast<zI32>(KeyCode::Digit0) + i);

		// -----------------------------
		// Function keys
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
		// Control keys
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
		// Arrow keys
		// -----------------------------
		arr[0x26] = KeyCode::ArrowUp;
		arr[0x28] = KeyCode::ArrowDown;
		arr[0x25] = KeyCode::ArrowLeft;
		arr[0x27] = KeyCode::ArrowRight;

		// -----------------------------
		// Modifiers
		// -----------------------------
		arr[0xA0] = KeyCode::LeftShift;  arr[0xA1] = KeyCode::RightShift;
		arr[0xA2] = KeyCode::LeftCtrl;   arr[0xA3] = KeyCode::RightCtrl;
		arr[0xA4] = KeyCode::LeftAlt;    arr[0xA5] = KeyCode::RightAlt;

		// -----------------------------
		// Locks
		// -----------------------------
		arr[0x14] = KeyCode::CapsLock;
		arr[0x90] = KeyCode::NumLock;
		arr[0x91] = KeyCode::ScrollLock;

		// -----------------------------
		// NumPad
		// -----------------------------
		arr[0x60] = KeyCode::Numpad0; arr[0x61] = KeyCode::Numpad1; arr[0x62] = KeyCode::Numpad2;
		arr[0x63] = KeyCode::Numpad3; arr[0x64] = KeyCode::Numpad4; arr[0x65] = KeyCode::Numpad5;
		arr[0x66] = KeyCode::Numpad6; arr[0x67] = KeyCode::Numpad7; arr[0x68] = KeyCode::Numpad8;
		arr[0x69] = KeyCode::Numpad9;
		arr[0x6B] = KeyCode::NumpadAdd; arr[0x6D] = KeyCode::NumpadSubtract;
		arr[0x6A] = KeyCode::NumpadMultiply; arr[0x6F] = KeyCode::NumpadDivide;
		arr[0x6E] = KeyCode::NumpadDecimal;
		// NumPad Enter – расширенный Enter (E0)
		arr[0x0D] = KeyCode::NumpadEnter;

		// -----------------------------
		// Symbols
		// -----------------------------
		arr[0xBD] = KeyCode::Minus;    arr[0xBB] = KeyCode::Equals;
		arr[0xDB] = KeyCode::LeftBracket; arr[0xDD] = KeyCode::RightBracket;
		arr[0xDC] = KeyCode::Backslash;  arr[0xBA] = KeyCode::Semicolon;
		arr[0xDE] = KeyCode::Apostrophe; arr[0xBC] = KeyCode::Comma;
		arr[0xBE] = KeyCode::Period;     arr[0xBF] = KeyCode::Slash;
		arr[0xC0] = KeyCode::Grave;

		// -----------------------------
		// System / special
		// -----------------------------
		arr[0x2C] = KeyCode::PrintScreen;
		arr[0x13] = KeyCode::Pause;
		arr[0x5D] = KeyCode::Menu;

		return arr;
		}();

	export constexpr KeyCode TranslateMSWinKey(UINT vk, bool e0)
	{
		switch (vk)
		{
		case 0x0D: return e0 ? KeyCode::NumpadEnter : KeyCode::Enter;
		case 0x11: return e0 ? KeyCode::RightCtrl : KeyCode::LeftCtrl;
		case 0x12: return e0 ? KeyCode::RightAlt : KeyCode::LeftAlt;
		case 0x5B: return e0 ? KeyCode::LeftMeta : KeyCode::Unknown; // Win key
		case 0x5C: return e0 ? KeyCode::RightMeta : KeyCode::Unknown; // Win key
		default:
			if (vk < MSWinVirtualKeyMap.size())
				return MSWinVirtualKeyMap[vk];
			return KeyCode::Unknown;
		}
	}
}
#endif