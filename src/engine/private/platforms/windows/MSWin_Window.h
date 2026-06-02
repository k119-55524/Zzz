#pragma once

#if defined(_WIN64)

#include "IWindow.h"

namespace zzz::engine
{
	class MSWin_Window final : public IWindow
	{
	};
}
#endif // defined(_WIN64)