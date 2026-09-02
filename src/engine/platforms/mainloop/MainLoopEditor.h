#pragma once

#include "core/utils/Defines.h"

#if defined(Z_EDITOR)

#include "MainLoopCommon.h"

namespace zzz::engine
{
	class MainLoop_Editor : public MainLoopBase
	{
	public:
		using MainLoopBase::MainLoopBase;
		virtual void Run() override { /* Пустая заглушка для редактора */ }
	};
}

#endif // defined(Z_EDITOR)
