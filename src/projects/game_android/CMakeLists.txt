find_package(game-activity REQUIRED CONFIG)

add_library(game_android SHARED)

set(Z_ASSET_BUILD_DIRS "")
set(ASSETS_CONFIG_JSON "${CMAKE_CURRENT_SOURCE_DIR}/assets_config.json")

if(EXISTS "${ASSETS_CONFIG_JSON}")
    file(READ "${ASSETS_CONFIG_JSON}" JSON_CONTENT)
    string(JSON DIRS_COUNT ERROR_VARIABLE JSON_ERR LENGTH "${JSON_CONTENT}" "active_build_directories")
    if(NOT JSON_ERR AND DIRS_COUNT GREATER 0)
        math(EXPR DIRS_MAX "${DIRS_COUNT} - 1")
        foreach(INDEX RANGE ${DIRS_MAX})
            string(JSON B_DIR GET "${JSON_CONTENT}" "active_build_directories" ${INDEX})
            if(B_DIR MATCHES "^%([^%]+)%(.*)")
                set(ENV_VAR_NAME "${CMAKE_MATCH_1}")
                set(ENV_VAR_REST "${CMAKE_MATCH_2}")
                if(DEFINED ENV{${ENV_VAR_NAME}})
                    set(B_DIR "$ENV{${ENV_VAR_NAME}}${ENV_VAR_REST}")
                endif()
            endif()
            list(APPEND Z_ASSET_BUILD_DIRS "${B_DIR}")
        endforeach()
    endif()
endif()

list(REMOVE_DUPLICATES Z_ASSET_BUILD_DIRS)

set(SCRIPT_HPPS "")
set(SCRIPT_CPPS "")

foreach(BUILD_DIR ${Z_ASSET_BUILD_DIRS})
    if(EXISTS "${BUILD_DIR}/Scripts.cmake")
        include("${BUILD_DIR}/Scripts.cmake")
        foreach(SRC ${GAME_SCRIPT_SOURCES})
            if(EXISTS "${SRC}")
                list(APPEND SCRIPT_CPPS "${SRC}")
            else()
                message(WARNING "Asset script source not found: '${SRC}'. Please rebuild assets in Assets Builder.")
            endif()
        endforeach()
        foreach(HDR ${GAME_SCRIPT_HEADERS})
            if(EXISTS "${HDR}")
                list(APPEND SCRIPT_HPPS "${HDR}")
            endif()
        endforeach()
        target_include_directories(game_android PRIVATE ${GAME_SCRIPT_INCLUDES})
    endif()

    if(EXISTS "${BUILD_DIR}/assets")
        file(COPY "${BUILD_DIR}/assets/" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/app/src/main/assets")
    endif()
endforeach()

target_precompile_headers(game_android PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:pch/pch.h>"
)

target_include_directories(game_android PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine/scene"
    "${CMAKE_SOURCE_DIR}/src/core/userscripts"
    "${CMAKE_SOURCE_DIR}/src/core/userscripts/base_script"
)

target_sources(game_android PRIVATE
    pch/pch.h
    pch/pch.cpp
    src/android_main.h
    src/android_main.cpp
    ${SCRIPT_CPPS}
)

target_link_libraries(game_android PRIVATE
    engine_lib
    logger_lib

    # Это магическая строка, которая решит UnsatisfiedLinkError
    -Wl,--whole-archive game-activity::game-activity_static -Wl,--no-whole-archive

    android
    log
)

target_compile_features(game_android PUBLIC cxx_std_23)
target_compile_definitions(game_android PRIVATE Z_TARGET_NAME="game_android")
set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)



