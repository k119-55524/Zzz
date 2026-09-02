#pragma once

#include "core/utils/Defines.h"

#if defined(Z_MACOS)

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

#endif // defined(Z_MACOS)
