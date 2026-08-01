
#include <thread>
#include <common/Common.h>
#include <benchmark/benchmark.h>

using namespace zzz;

static void BM_DoubleBufferedVector_Push(benchmark::State& state) {
	DoubleBufferedVector<int> vec(1000000);
	for (auto _ : state) {
		vec.Push(42);
	}
}
BENCHMARK(BM_DoubleBufferedVector_Push);

static void BM_DoubleBufferedVector_MultiThreadedPush(benchmark::State& state) {
	DoubleBufferedVector<int> vec(1000000);
	for (auto _ : state) {
		vec.Push(42);
	}
}
BENCHMARK(BM_DoubleBufferedVector_MultiThreadedPush)->Threads(4);

static void BM_DoubleBufferedVector_Swap(benchmark::State& state) {
	DoubleBufferedVector<int> vec(100);
	vec.Push(42);
	for (auto _ : state) {
		vec.SwapAndGetReadBuffer();
		vec.Push(42);
	}
}
BENCHMARK(BM_DoubleBufferedVector_Swap);