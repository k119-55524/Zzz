#include "pch.h"

#include "engine.h"

namespace zzz::engine
{
	Engine::Engine() :
		initState{zzz::InitNot}
	{
	}

	std::expected<void, std::wstring> Engine::Initialize()
	{
		std::lock_guard lock(initMutex);

		if (initState != zzz::InitNot)
		{
			DOut(L">>>>> [Engine::Initialize()]. Engine is already initialized or in the process of initialization.");
			return std::unexpected(L"Engine is already initialized or in the process of initialization.");
		}

		DOut(L">>>>> [Engine::Initialize()]. Engine initialized.");

		initState = zzz::InitOK;
		return {};
	}
}
