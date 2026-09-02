#pragma once

#include "core/utils/Defines.h"

#if defined(Z_WINDOWS)

#include "InputBase.h"

namespace zzz::engine
{
	class InputMSWindows final : public InputBase
	{
	public:
		InputMSWindows() = default;
		~InputMSWindows()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;
		bool ProcessMessage(const NativeMsg& msg) ;

	protected:
		int InitRawInput(HWND hWnd);
		void OnRawInput(HRAWINPUT hRawInput);
		void HandleRawMouse(const RAWMOUSE& mouse);
		void HandleRawKeyboard(const RAWKEYBOARD& kb);

		constexpr KeyCode TranslateMSWinKey(UINT vk, bool e0, UINT makeCode)
		{
			switch (vk)
			{
			case 0x0D: return e0 ? KeyCode::NumpadEnter : KeyCode::Enter;
			case 0x10: return makeCode == 0x36 ? KeyCode::RightShift : KeyCode::LeftShift;
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
	};
}
#endif // defined(Z_WINDOWS)
