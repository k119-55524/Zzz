include(FetchContent)

# -----------------------------------------------------------------------------
# Google Test
# -----------------------------------------------------------------------------
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

# Для Windows: чтобы GTest собирался с теми же настройками CRT, что и наш проект
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# -----------------------------------------------------------------------------
# Tests Target
# -----------------------------------------------------------------------------
add_executable(ZzzTests
    DummyTest.cpp
    # EventTests.cpp будет добавлен позже, как мы и договаривались
)

target_link_libraries(ZzzTests
    PRIVATE
        GTest::gtest_main
        engine_lib
)

# -----------------------------------------------------------------------------
# CTest / IDE Integration
# -----------------------------------------------------------------------------
include(GoogleTest)
gtest_discover_tests(ZzzTests)
