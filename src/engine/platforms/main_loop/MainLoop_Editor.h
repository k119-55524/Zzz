#pragma once
#include "MainLoop_Common.h"

namespace zzz::engine
{
	class MainLoop_Editor : public MainLoopBase
	{
	public:
		using MainLoopBase::MainLoopBase;
		virtual void Run() override { /* Пустая заглушка для редактора */ }
	};
}
