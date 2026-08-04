add_executable(game_linux)

set(Z_ASSET_BUILD_DIRS "")
set(ASSETS_CONFIG_JSON "${CMAKE_CURRENT_SOURCE_DIR}/assets_config.json")

if(EXISTS "${ASSETS_CONFIG_JSON}")
    file(READ "${ASSETS_CONFIG_JSON}" JSON_CONTENT)
    string(JSON DIRS_COUNT ERROR_VARIABLE JSON_ERR LENGTH "${JSON_CONTENT}" "active_build_directories")
    if(NOT JSON_ERR AND DIRS_COUNT GREATER 0)
        math(EXPR DIRS_MAX "${DIRS_COUNT} - 1")
        foreach(INDEX RANGE ${DIRS_MAX})
            string(JSON B_DIR GET "${JSON_CONTENT}" "active_build_directories" ${INDEX})
            list(APPEND Z_ASSET_BUILD_DIRS "${B_DIR}")
        endforeach()
    endif()
endif()

if(Z_GAME_PROJECT_DIR)
    get_filename_component(FALLBACK_BUILD_DIR "${Z_GAME_PROJECT_DIR}/../zzz_assets_test_000_build" ABSOLUTE)
    if(EXISTS "${FALLBACK_BUILD_DIR}")
        list(APPEND Z_ASSET_BUILD_DIRS "${FALLBACK_BUILD_DIR}")
    endif()
endif()

list(REMOVE_DUPLICATES Z_ASSET_BUILD_DIRS)

set(SCRIPT_HPPS "")
set(SCRIPT_CPPS "")

foreach(BUILD_DIR ${Z_ASSET_BUILD_DIRS})
    if(EXISTS "${BUILD_DIR}/Scripts.cmake")
        include("${BUILD_DIR}/Scripts.cmake")
        list(APPEND SCRIPT_CPPS ${GAME_SCRIPT_SOURCES})
        list(APPEND SCRIPT_HPPS ${GAME_SCRIPT_HEADERS})
        target_include_directories(game_linux PRIVATE ${GAME_SCRIPT_INCLUDES})
    endif()

    if(EXISTS "${BUILD_DIR}/assets")
        add_custom_command(TARGET game_linux POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${BUILD_DIR}/assets"
            "$<TARGET_FILE_DIR:game_linux>/assets"
        )
    endif()
endforeach()

set(REGISTER_SCRIPTS_CONTENT "// RegisterAllScripts.cpp — генерируется автоматически CMake\n#include <ScriptRegistry.h>\n\n")
set(REGISTER_SCRIPT_LINES "")
foreach(HPP_FILE ${SCRIPT_HPPS})
    get_filename_component(CLASS_NAME "${HPP_FILE}" NAME_WE)
    get_filename_component(SCRIPT_DIR "${HPP_FILE}" DIRECTORY)
    set(SCRIPT_NAMESPACE "")
    set(META_FILE "${SCRIPT_DIR}/${CLASS_NAME}.meta")
    if(EXISTS "${META_FILE}")
        file(READ "${META_FILE}" META_CONTENT)
        string(REGEX MATCH "namespace[ \t]*=[ \t]*\"([^\"]*)\"" META_NAMESPACE_MATCH "${META_CONTENT}")
        if(META_NAMESPACE_MATCH)
            set(SCRIPT_NAMESPACE "${CMAKE_MATCH_1}")
        endif()
    endif()

    if(SCRIPT_NAMESPACE STREQUAL "")
        set(QUALIFIED_NAME "${CLASS_NAME}")
    else()
        set(QUALIFIED_NAME "${SCRIPT_NAMESPACE}::${CLASS_NAME}")
    endif()

    string(APPEND REGISTER_SCRIPTS_CONTENT "#include \"${HPP_FILE}\"\n")
    string(APPEND REGISTER_SCRIPT_LINES "    zzz::core::ScriptRegistry::Register<${QUALIFIED_NAME}>(\"${QUALIFIED_NAME}\");\n")
endforeach()

string(APPEND REGISTER_SCRIPTS_CONTENT "\nvoid RegisterAllScripts()\n{\n${REGISTER_SCRIPT_LINES}}\n")
set(GENERATED_REGISTER_CPP "${CMAKE_CURRENT_BINARY_DIR}/RegisterAllScripts.cpp")
file(WRITE "${GENERATED_REGISTER_CPP}" "${REGISTER_SCRIPTS_CONTENT}")

target_precompile_headers(game_linux PRIVATE
    pch/pch.h
)

target_include_directories(game_linux PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine/scene"
    "${CMAKE_SOURCE_DIR}/src/core/userscripts"
    "${CMAKE_SOURCE_DIR}/src/core/userscripts/base_script"
)

target_sources(game_linux
    PRIVATE
        pch/pch.h
        pch/pch.cpp
        main.h
        main.cpp
        ${SCRIPT_CPPS}
        ${GENERATED_REGISTER_CPP}
)

target_link_libraries(game_linux
    PRIVATE
        engine_lib
        logger_lib
)

target_compile_features(game_linux PUBLIC cxx_std_23)
target_compile_definitions(game_linux PRIVATE Z_TARGET_NAME="game_linux")
set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)



