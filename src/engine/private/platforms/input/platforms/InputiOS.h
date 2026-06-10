#pragma once


#include "../Input.h"

namespace zzz::engine
{
	class InputiOS final
	{
	public:
		InputiOS() = default;
		~InputiOS()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;
		bool ProcessMessage(const NativeMsg& nativeMsg) ;
	};
}
