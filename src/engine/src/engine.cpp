module;

#include "pch.h"

module engine;

import logger;

using namespace zzz::logger;

namespace zzz::engine
{
	Engine::Engine() :
		initState{InitNot}
	{
	}

	std::expected<void, std::wstring> Engine::Initialize()
	{
		std::lock_guard lock(initMutex);

		if (initState != InitNot)
		{
			DOut(L">>>>> [Engine::Initialize()]. Engine is already initialized or in the process of initialization.");
			return std::unexpected(L"Engine is already initialized or in the process of initialization.");
		}

		DOut(L">>>>> [Engine::Initialize()]. Engine initialized.");

		initState = InitOK;
		return {};
	}
}