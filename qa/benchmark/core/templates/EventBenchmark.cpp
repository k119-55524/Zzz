
#include <benchmark/benchmark.h>
#include "../../src/engine/private/core/templates/Event.h"

using namespace zzz::engine;

static void SymmetricRangeArgs(benchmark::internal::Benchmark* b) {
	const std::vector<int> args = { 1, 8, 64, 512, 1024 };
	for (int a : args) {
		b->Arg(a);
	}
}

static void BM_EventSubscribe(benchmark::State& state)
{
	for (auto _ : state)
	{
		Event<void> evt;
		for (int i = 0; i < state.range(0); ++i)
		{
			evt.SubscribeStaticUnsafe([]() { benchmark::DoNotOptimize(1); });
		}
	}
}
BENCHMARK(BM_EventSubscribe)->Name("[UNSAFE] 1. Subscribe")->Apply(SymmetricRangeArgs);

static void BM_EventInvoke(benchmark::State& state)
{
	Event<int> evt;
	for (int i = 0; i < state.range(0); ++i)
	{
		evt.SubscribeStaticUnsafe([](int x) { benchmark::DoNotOptimize(x); });
	}

	for (auto _ : state)
	{
		evt(42);
	}
}
BENCHMARK(BM_EventInvoke)->Name("[UNSAFE] 2. Invoke(int)")->Apply(SymmetricRangeArgs);

static void BM_EventInvokeNoArgs(benchmark::State& state)
{
	Event<void> evt;
	for (int i = 0; i < state.range(0); ++i)
	{
		evt.SubscribeStaticUnsafe([]() { benchmark::DoNotOptimize(1); });
	}

	for (auto _ : state)
	{
		evt();
	}
}
BENCHMARK(BM_EventInvokeNoArgs)->Name("[UNSAFE] 3. Invoke()")->Apply(SymmetricRangeArgs);

static void BM_EventAutoUnsubscribe(benchmark::State& state)
{
	for (auto _ : state)
	{
		state.PauseTiming();
		Event<void> evt;
		for (int i = 0; i < state.range(0); ++i)
		{
			auto ctx = std::make_shared<int>(0);
			evt.SubscribeUnsafe(ctx, []() { benchmark::DoNotOptimize(1); });
		}
		state.ResumeTiming();

		evt();
	}
}
BENCHMARK(BM_EventAutoUnsubscribe)->Name("[UNSAFE] 4. AutoUnsubscribe")->Apply(SymmetricRangeArgs);
