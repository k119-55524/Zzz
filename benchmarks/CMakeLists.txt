include(FetchContent)

# -----------------------------------------------------------------------------
# Google Benchmark
# -----------------------------------------------------------------------------
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.8.3
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googlebenchmark)

# -----------------------------------------------------------------------------
# Benchmarks Target
# -----------------------------------------------------------------------------
add_executable(ZzzBenchmarks
    DummyBenchmark.cpp
)

target_link_libraries(ZzzBenchmarks
    PRIVATE
        benchmark::benchmark_main
        engine_lib
)
