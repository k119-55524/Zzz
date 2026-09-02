#pragma once

#include "core/utils/Defines.h"

#if defined(Z_ANDROID)

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

#endif // defined(Z_ANDROID)
