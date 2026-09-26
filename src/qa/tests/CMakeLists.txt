include(FetchContent)

# -----------------------------------------------------------------------------
# Google Test
# -----------------------------------------------------------------------------
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        ${Z_GTEST_VERSION}
  UPDATE_DISCONNECTED TRUE
)

# Для Windows: чтобы GTest собирался с теми же настройками CRT, что и наш проект
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

# -----------------------------------------------------------------------------
# Tests Target
# -----------------------------------------------------------------------------
set(TEST_SOURCES "")

if(Z_TESTS_ENABLE_COMMON)
    file(GLOB_RECURSE TEST_CPP_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/math/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/core/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/gapi/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/texture_processor/*.cpp"
    )
    list(APPEND TEST_SOURCES ${TEST_CPP_FILES})
endif()

if(TEST_SOURCES)
    add_executable(EngineTests
        ${TEST_SOURCES}
    )

    target_link_libraries(EngineTests
        PRIVATE
            GTest::gtest_main
            engine_lib
            core_lib
            texture_processor
    )

    # -----------------------------------------------------------------------------
    # CTest / IDE Integration
    # -----------------------------------------------------------------------------
    include(GoogleTest)
    gtest_discover_tests(EngineTests)
else()
    message(FATAL_ERROR "ОШИБКА: Глобальная сборка тестов (Z_ADD_PROJECT_TESTS_IN_BUILD) включена, но ни один модуль тестов (например, Z_TESTS_ENABLE_COMMON) не выбран! Включите хотя бы один модуль или выключите Z_ADD_PROJECT_TESTS_IN_BUILD.")
endif()
