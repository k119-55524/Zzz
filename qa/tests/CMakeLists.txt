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
set(TEST_SOURCES "")

if(Z_TESTS_ENABLE_TEMPLATES)
    file(GLOB_RECURSE TEMPLATES_SOURCES "core/templates/*.cpp")
    list(APPEND TEST_SOURCES ${TEMPLATES_SOURCES})
endif()

if(TEST_SOURCES)
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
else()
    message(FATAL_ERROR "ОШИБКА: Глобальная сборка тестов (Z_ADD_PROJECT_TESTS_IN_BUILD) включена, но ни один модуль тестов (например, Z_TESTS_ENABLE_TEMPLATES) не выбран! Включите хотя бы один модуль или выключите Z_ADD_PROJECT_TESTS_IN_BUILD.")
endif()
