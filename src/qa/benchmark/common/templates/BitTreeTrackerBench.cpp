#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include "core/containers/BitTreeTracker.h"

using namespace zzz::core;

// 1. Бенчмарк пометки битов (Set)
static void BM_BitTreeTracker_Set(benchmark::State& state)
{
	const uint32_t capacity = static_cast<uint32_t>(state.range(0));
	BitTreeTracker tracker(capacity);

	uint32_t idx = 0;
	for (auto _ : state)
	{
		tracker.Set(idx);
		idx = (idx + 7919) % static_cast<uint32_t>(capacity);
	}
}
BENCHMARK(BM_BitTreeTracker_Set)->Arg(1000)->Arg(10000)->Arg(100000);

// 2. Бенчмарк сбора разряженных изменений (Sparse Traversal: 1% грязных объектов)
static void BM_BitTreeTracker_GetDirtyIndices_Sparse(benchmark::State& state)
{
	const uint32_t capacity = static_cast<uint32_t>(state.range(0));
	BitTreeTracker tracker(capacity);

	for (size_t i = 0; i < capacity; i += 100)
	{
		tracker.Set(static_cast<uint32_t>(i));
	}

	for (auto _ : state)
	{
		auto dirty = tracker.GetDirtyIndices();
		benchmark::DoNotOptimize(dirty.data());
		benchmark::DoNotOptimize(dirty.size());
	}
}
BENCHMARK(BM_BitTreeTracker_GetDirtyIndices_Sparse)->Arg(10000)->Arg(100000);

// 3. Бенчмарк сбора плотных изменений (Dense Traversal: 50% объектов)
static void BM_BitTreeTracker_GetDirtyIndices_Dense(benchmark::State& state)
{
	const uint32_t capacity = static_cast<uint32_t>(state.range(0));
	BitTreeTracker tracker(capacity);

	for (size_t i = 0; i < capacity; i += 2)
	{
		tracker.Set(static_cast<uint32_t>(i));
	}

	for (auto _ : state)
	{
		auto dirty = tracker.GetDirtyIndices();
		benchmark::DoNotOptimize(dirty.data());
		benchmark::DoNotOptimize(dirty.size());
	}
}
BENCHMARK(BM_BitTreeTracker_GetDirtyIndices_Dense)->Arg(10000)->Arg(50000);

// 4. Бенчмарк сбора пустых изменений (0 грязных объектов)
static void BM_BitTreeTracker_GetDirtyIndices_Empty(benchmark::State& state)
{
	const uint32_t capacity = static_cast<uint32_t>(state.range(0));
	BitTreeTracker tracker(capacity);

	for (auto _ : state)
	{
		auto dirty = tracker.GetDirtyIndices();
		benchmark::DoNotOptimize(dirty.data());
		benchmark::DoNotOptimize(dirty.size());
	}
}
BENCHMARK(BM_BitTreeTracker_GetDirtyIndices_Empty)->Arg(10000)->Arg(100000);

// 5. Бенчмарк покадровой подготовки и очистки (Prepare)
static void BM_BitTreeTracker_Prepare(benchmark::State& state)
{
	const uint32_t capacity = static_cast<uint32_t>(state.range(0));
	BitTreeTracker tracker(capacity);
	tracker.Set(10);
	tracker.Set(static_cast<uint32_t>(capacity - 1));

	for (auto _ : state)
	{
		state.PauseTiming();
		tracker.Set(10);
		tracker.Set(static_cast<uint32_t>(capacity - 1));
		state.ResumeTiming();

		tracker.Prepare(capacity);
		benchmark::ClobberMemory();
	}
}
BENCHMARK(BM_BitTreeTracker_Prepare)->Arg(10000)->Arg(100000);
