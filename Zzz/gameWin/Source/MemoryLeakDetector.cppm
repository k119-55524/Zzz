
module;

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

export module MemoryLeakDetector;

import DebugOutput;

using namespace zzz::logger;

#ifdef _DEBUG
#include <crtdbg.h>

export namespace debug::crt
{
	struct LeakContext
	{
		_CrtMemState begin{};
	};

	template<typename... Args>
	inline void Begin(LeakContext& ctx, Args... breakAllocs) noexcept
	{
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF);

		if constexpr (sizeof...(breakAllocs) > 0)
			(_CrtSetBreakAlloc(static_cast<long>(breakAllocs)), ...);

		_CrtMemCheckpoint(&ctx.begin);
	}

	inline int End(LeakContext& ctx) noexcept
	{
		_CrtMemState end{}, diff{};
		_CrtMemCheckpoint(&end);

		if (_CrtMemDifference(&diff, &ctx.begin, &end))
		{
			DebugOutputLite(L"\n+================================================+");
			DebugOutputLite(L">>>>> MEMORY LEAKS DETECTED   :(");
			DebugOutputLite(L">>>>> DUMP:");
			_CrtMemDumpStatistics(&diff);
			DebugOutputLite(L"\n>>>>> MEMORY LEAKS LIST:");
			_CrtMemDumpAllObjectsSince(&ctx.begin);
			DebugOutputLite(L"+================================================+\n");

			return -1; // код ошибки
		}

		DebugOutputLite(L">>>>> NO MEMORY LEAKS. :)");
		return 0;
	}
}
#endif