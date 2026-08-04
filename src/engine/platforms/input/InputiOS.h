#pragma once


#include "InputBase.h"

namespace zzz::engine
{
	class InputiOS final : public InputBase
	{
	public:
		InputiOS() = default;
		~InputiOS()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;
		bool ProcessMessage(const NativeMsg& nativeMsg) ;
	};
}
