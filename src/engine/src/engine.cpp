module;

#include "pch.h"

module zzz.engine;

import logger;

using namespace zzz::logger;

namespace zzz::engine
{
	void Engine::Initialize()
	{
		DOut(L">>>>> [Engine::Initialize()]. Engine initialized.");
	}
}