#pragma once


#include "Input_Common.h"

namespace zzz::engine
{
	class InputMSWindows final
	{
	public:
		InputMSWindows() = default;
		~InputMSWindows()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;
		bool ProcessMessage(const NativeMsg& nativeMsg) ;
	};
}
