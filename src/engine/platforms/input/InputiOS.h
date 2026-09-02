#pragma once

#include "core/utils/Defines.h"

#if defined(Z_IOS)

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

#endif // defined(Z_IOS)
