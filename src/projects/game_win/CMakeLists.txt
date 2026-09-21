add_executable(game_win WIN32)

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
    # 1. Подключаем автогенерируемый Сборщиком файл скриптов и C++ регистрации RegisterAllScripts.cpp
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
        target_include_directories(game_win PRIVATE ${GAME_SCRIPT_INCLUDES})
    endif()

    # 2. Копирование папки с бинарным пакетом ресурсов assets/при сборке целевого проекта
    if(EXISTS "${BUILD_DIR}/assets")
        add_custom_command(TARGET game_win POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${BUILD_DIR}/assets"
            "$<TARGET_FILE_DIR:game_win>/assets"
        )
    endif()
endforeach()

# RegisterAllScripts.cpp сгенерирован сборщиком ассетов в destinationPath и включен через GAME_SCRIPT_SOURCES в Scripts.cmake

target_precompile_headers(game_win PRIVATE
    pch/pch.h
)

target_include_directories(game_win PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine"
    "${CMAKE_SOURCE_DIR}/src/engine/scene"
    "${CMAKE_SOURCE_DIR}/src/core/userscripts"
    "${CMAKE_SOURCE_DIR}/src/core/userscripts/base_script"
)

target_sources(game_win PRIVATE
    pch/pch.h
    pch/pch.cpp
    main.h
    main.cpp
    resource/app.rc
    ${SCRIPT_CPPS}
)

target_link_libraries(game_win PRIVATE
    engine_lib
    core_lib
)

target_compile_features(game_win PUBLIC cxx_std_23)
target_compile_definitions(game_win PRIVATE Z_TARGET_NAME="game_win")
set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
