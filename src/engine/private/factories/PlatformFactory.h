#pragma once

#include "PlatformFactory.h"
namespace zzz::engine
{
#if defined(_WIN64)
#include "../platforms/windows/MSWin_Window.h"
	typedef zzz::engine::MSWin_Window Window;
#endif // defined(_WIN64)

	class PlatformFactory final
	{
	};
}