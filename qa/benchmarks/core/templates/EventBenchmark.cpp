
#include <benchmark/benchmark.h>
#include "../../src/engine/private/core/templates/Event.h"

using namespace zzz::engine;

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
static void SymmetricRangeArgs(benchmark::internal::Benchmark* b) {
	const std::vector<int> args = { 1, 8, 64, 512, 4096, 512, 64, 8, 1 };
	for (int a : args) {
		b->Arg(a)->Iterations(100); // 100 итераций достаточно для фиксации времени
	}
}

BENCHMARK(BM_EventSubscribe)->Apply(SymmetricRangeArgs);

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
BENCHMARK(BM_EventInvoke)->Apply(SymmetricRangeArgs);

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
BENCHMARK(BM_EventInvokeNoArgs)->Apply(SymmetricRangeArgs);
