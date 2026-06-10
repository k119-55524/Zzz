#pragma once


#include "../Input_Common.h"

namespace zzz::engine
{
	class InputLinux final
	{
	public:
		InputLinux() = default;
		~InputLinux()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;
		bool ProcessMessage(const NativeMsg& nativeMsg) ;
	};
}
