
#include <benchmark/benchmark.h>
#include <core/events/Event.h>

using namespace zzz::engine;

static std::shared_ptr<int> g_ctx = std::make_shared<int>(0);

static void SymmetricRangeArgs(benchmark::Benchmark* b) {
	const std::vector<int> args = { 1, 8, 64, 512, 1024 };
	for (int a : args) {
		b->Arg(a);
	}
}

static void BM_EventSubscribe(benchmark::State& state)
{
	for (auto _ : state)
	{
		Event<> evt;
		for (int i = 0; i < state.range(0); ++i)
		{
			evt.Subscribe(g_ctx, []() { benchmark::DoNotOptimize(1); });
		}
	}
}
BENCHMARK(BM_EventSubscribe)->Name("[UNSAFE] 1. Subscribe")->Apply(SymmetricRangeArgs);

static void BM_EventInvoke(benchmark::State& state)
{
	Event<int> evt;
	for (int i = 0; i < state.range(0); ++i)
	{
		evt.Subscribe(g_ctx, [](int x) { benchmark::DoNotOptimize(x); });
	}

	for (auto _ : state)
	{
		evt(42);
	}
}
BENCHMARK(BM_EventInvoke)->Name("[UNSAFE] 2. Invoke(int)")->Apply(SymmetricRangeArgs);

static void BM_EventInvokeNoArgs(benchmark::State& state)
{
	Event<> evt;
	for (int i = 0; i < state.range(0); ++i)
	{
		evt.Subscribe(g_ctx, []() { benchmark::DoNotOptimize(1); });
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
		Event<> evt;
		for (int i = 0; i < state.range(0); ++i)
		{
			auto ctx = std::make_shared<int>(0);
			evt.Subscribe(ctx, []() { benchmark::DoNotOptimize(1); });
		}
		state.ResumeTiming();

		evt();
	}
}
BENCHMARK(BM_EventAutoUnsubscribe)->Name("[UNSAFE] 4. AutoUnsubscribe")->Apply(SymmetricRangeArgs);

static void BM_OrderedEventCleanup(benchmark::State& state)
{
	for (auto _ : state)
	{
		state.PauseTiming();
		Event<> evt;
		std::vector<std::shared_ptr<int>> contexts;
		for (int i = 0; i < state.range(0); ++i)
		{
			auto ctx = std::make_shared<int>(0);
			contexts.push_back(ctx);
			evt.Subscribe(ctx, []() { benchmark::DoNotOptimize(1); });
		}
		
		// "Убиваем" половину подписчиков
		for (int i = 0; i < state.range(0); i += 2)
		{
			contexts[i].reset();
		}
		state.ResumeTiming();

		// Этот вызов спровоцирует O(N) cleanup через std::remove_if
		evt();
	}
}
BENCHMARK(BM_OrderedEventCleanup)->Name("[SAFE] 5. Ordered Cleanup (remove_if)")->Apply(SymmetricRangeArgs);

static void BM_UnorderedEventCleanup(benchmark::State& state)
{
	for (auto _ : state)
	{
		state.PauseTiming();
		UnorderedEvent<> evt;
		std::vector<std::shared_ptr<int>> contexts;
		for (int i = 0; i < state.range(0); ++i)
		{
			auto ctx = std::make_shared<int>(0);
			contexts.push_back(ctx);
			evt.Subscribe(ctx, []() { benchmark::DoNotOptimize(1); });
		}
		
		// "Убиваем" половину подписчиков
		for (int i = 0; i < state.range(0); i += 2)
		{
			contexts[i].reset();
		}
		state.ResumeTiming();

		// Этот вызов спровоцирует O(1) cleanup через Swap-and-Pop
		evt();
	}
}
BENCHMARK(BM_UnorderedEventCleanup)->Name("[SAFE] 6. Unordered Cleanup (Swap-and-Pop)")->Apply(SymmetricRangeArgs);

