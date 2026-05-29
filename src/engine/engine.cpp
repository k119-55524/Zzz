#include "pch.h"

#include "engine.h"

namespace zzz::engine
{
	Engine::Engine() :
		initState{zzz::InitNot}
	{
	}

	std::expected<void, std::string> Engine::Initialize()
	{
		std::lock_guard lock(initMutex);

		if (initState != zzz::InitNot)
		{
			DOut(">>>>> [Engine::Initialize()]. Engine is already initialized or in the process of initialization.");
			return std::unexpected("Engine is already initialized or in the process of initialization.");
		}

		DOut(">>>>> [Engine::Initialize()]. Engine initialized.");

		initState = zzz::InitOK;
		return {};
	}
}
