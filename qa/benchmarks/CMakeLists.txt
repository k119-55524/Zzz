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
file(GLOB_RECURSE BENCH_SOURCES "*.cpp")

if(NOT Z_TESTS_BENCH_ENABLE_TEMPLATES)
    list(FILTER BENCH_SOURCES EXCLUDE REGEX ".*/templates/.*")
endif()

if(NOT BENCH_SOURCES)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
    set(BENCH_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
endif()

add_executable(TestBenchmarks
    ${BENCH_SOURCES}
)

target_link_libraries(TestBenchmarks
    PRIVATE
        benchmark::benchmark_main
        engine_lib
)
