#pragma once

#include "core/utils/Defines.h"

#if defined(Z_LINUX)

#include "InputBase.h"

namespace zzz::engine
{
	class InputLinux final : public InputBase
	{
	public:
		InputLinux() = default;
		~InputLinux()  = default;

		[[nodiscard]] std::expected<void, std::string> Initialize() ;
		bool ProcessMessage(const NativeMsg& nativeMsg) ;
	};
}

#endif // defined(Z_LINUX)
