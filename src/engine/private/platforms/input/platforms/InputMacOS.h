#pragma once


#include "../Input_Common.h"

namespace zzz::engine
{
	class InputMacOS final
	{
	public:
		InputMacOS() = default;
		~InputMacOS()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;
		bool ProcessMessage(const NativeMsg& nativeMsg) ;
	};
}
