
#include <benchmark/benchmark.h>
#include "../../src/engine/private/core/templates/EventThreadSafe.h"

using namespace zzz::engine;

static void SymmetricRangeArgs(benchmark::internal::Benchmark* b) {
	const std::vector<int> args = { 1, 8, 64, 512, 1024 };
	for (int a : args) {
		b->Arg(a);
	}
}

static void BM_EventThreadSafeSubscribe(benchmark::State& state)
{
	for (auto _ : state)
	{
		EventThreadSafe<void> evt;
		for (int i = 0; i < state.range(0); ++i)
		{
			evt.SubscribeStaticSafe([]() { benchmark::DoNotOptimize(1); });
		}
	}
}
BENCHMARK(BM_EventThreadSafeSubscribe)->Name("[THREAD-SAFE] 1. Subscribe")->Apply(SymmetricRangeArgs);

static void BM_EventThreadSafeInvoke(benchmark::State& state)
{
	EventThreadSafe<int> evt;
	for (int i = 0; i < state.range(0); ++i)
	{
		evt.SubscribeStaticSafe([](int x) { benchmark::DoNotOptimize(x); });
	}

	for (auto _ : state)
	{
		evt(42);
	}
}
BENCHMARK(BM_EventThreadSafeInvoke)->Name("[THREAD-SAFE] 2. Invoke(int)")->Apply(SymmetricRangeArgs);

static void BM_EventThreadSafeInvokeNoArgs(benchmark::State& state)
{
	EventThreadSafe<void> evt;
	for (int i = 0; i < state.range(0); ++i)
	{
		evt.SubscribeStaticSafe([]() { benchmark::DoNotOptimize(1); });
	}

	for (auto _ : state)
	{
		evt();
	}
}
BENCHMARK(BM_EventThreadSafeInvokeNoArgs)->Name("[THREAD-SAFE] 3. Invoke()")->Apply(SymmetricRangeArgs);

static void BM_EventThreadSafeAutoUnsubscribe(benchmark::State& state)
{
	for (auto _ : state)
	{
		state.PauseTiming();
		EventThreadSafe<void> evt;
		for (int i = 0; i < state.range(0); ++i)
		{
			auto ctx = std::make_shared<int>(0);
			evt.SubscribeSafe(ctx, []() { benchmark::DoNotOptimize(1); });
		}
		state.ResumeTiming();

		evt();
	}
}
BENCHMARK(BM_EventThreadSafeAutoUnsubscribe)->Name("[THREAD-SAFE] 4. AutoUnsubscribe")->Apply(SymmetricRangeArgs);
