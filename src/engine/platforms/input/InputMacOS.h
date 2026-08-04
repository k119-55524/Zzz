#pragma once


#include "InputBase.h"

namespace zzz::engine
{
	class InputMacOS final : public InputBase
	{
	public:
		InputMacOS() = default;
		~InputMacOS()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;
		bool ProcessMessage(const NativeMsg& nativeMsg) ;
	};
}
