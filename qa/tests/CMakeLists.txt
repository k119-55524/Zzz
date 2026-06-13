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
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

if(NOT Z_TESTS_ENABLE_TEMPLATES)
    list(FILTER TEST_SOURCES EXCLUDE REGEX ".*/templates/.*")
endif()

if(NOT TEST_SOURCES)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp "")
    set(TEST_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/dummy.cpp)
endif()

add_executable(Tests
    ${TEST_SOURCES}
)

target_link_libraries(Tests
    PRIVATE
        GTest::gtest_main
        engine_lib
)

# -----------------------------------------------------------------------------
# CTest / IDE Integration
# -----------------------------------------------------------------------------
include(GoogleTest)
gtest_discover_tests(Tests)
