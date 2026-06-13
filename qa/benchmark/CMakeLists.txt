include(FetchContent)
# Trigger CMake reload for new benchmark files
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
set(BENCH_SOURCES "")

if(Z_TESTS_BENCH_ENABLE_TEMPLATES)
    file(GLOB_RECURSE TEMPLATES_BENCH_SOURCES "core/templates/*.cpp")
    list(APPEND BENCH_SOURCES ${TEMPLATES_BENCH_SOURCES})
endif()

if(BENCH_SOURCES)
    add_executable(Benchmark
        ${BENCH_SOURCES}
    )

    target_link_libraries(Benchmark
        PRIVATE
            benchmark::benchmark_main
            engine_lib
    )
else()
    message(FATAL_ERROR "ОШИБКА: Сборка бенчмарков (Z_ADD_PROJECT_BENCHMARKS_IN_BUILD) включена, но ни один модуль бенчмарков (например, Z_TESTS_BENCH_ENABLE_TEMPLATES) не выбран! Включите хотя бы один модуль или выключите Z_ADD_PROJECT_BENCHMARKS_IN_BUILD.")
endif()
