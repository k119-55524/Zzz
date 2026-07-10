include(FetchContent)
# Trigger CMake reload for new benchmark files
# -----------------------------------------------------------------------------
# Google Benchmark
# -----------------------------------------------------------------------------
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.9.5
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googlebenchmark)

# -----------------------------------------------------------------------------
# Benchmarks Target
# -----------------------------------------------------------------------------
set(BENCH_SOURCES "")

if(Z_TESTS_BENCH_ENABLE_COMMON)
    file(GLOB_RECURSE COMMON_BENCH_SOURCES "common/*.cpp")
    list(APPEND BENCH_SOURCES ${COMMON_BENCH_SOURCES})
endif()

if(BENCH_SOURCES)
    add_executable(EngineBenchmarks
        ${BENCH_SOURCES}
    )

    target_link_libraries(EngineBenchmarks
        PRIVATE
            benchmark::benchmark_main
            engine_lib
    )
else()
    message(FATAL_ERROR "ОШИБКА: Сборка бенчмарков (Z_ADD_PROJECT_BENCHMARKS_IN_BUILD) включена, но ни один модуль бенчмарков (например, Z_TESTS_BENCH_ENABLE_COMMON) не выбран! Включите хотя бы один модуль или выключите Z_ADD_PROJECT_BENCHMARKS_IN_BUILD.")
endif()
