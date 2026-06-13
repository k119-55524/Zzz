#include <benchmark/benchmark.h>

static void BM_Dummy(benchmark::State& state) {
    for (auto _ : state) {
        // Простой бенчмарк для проверки инфраструктуры
        benchmark::DoNotOptimize(1 + 1);
    }
}
BENCHMARK(BM_Dummy);
