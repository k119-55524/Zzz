#pragma once


#include "InputBase.h"

namespace zzz::engine
{
	class InputAndroid final : public InputBase
	{
	public:
		InputAndroid() = default;
		~InputAndroid()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;

		bool ProcessMessage(const NativeMsg& nativeMsg) ;

	};
}
