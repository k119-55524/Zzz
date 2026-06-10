#pragma once


#include "../Input_Common.h"

namespace zzz::engine
{
	class InputAndroid final
	{
	public:
		InputAndroid() = default;
		~InputAndroid()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;

		bool ProcessMessage(const NativeMsg& nativeMsg) ;

	};
}
