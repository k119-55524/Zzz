#pragma once


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
	};
}
