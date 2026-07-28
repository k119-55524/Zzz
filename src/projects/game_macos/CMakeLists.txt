add_executable(game_macos
    MACOSX_BUNDLE
    main.mm
)

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
set(ASSET_FILES "")

foreach(BUILD_DIR ${Z_ASSET_BUILD_DIRS})
    if(EXISTS "${BUILD_DIR}/Scripts.cmake")
        include("${BUILD_DIR}/Scripts.cmake")
        list(APPEND SCRIPT_CPPS ${GAME_SCRIPT_SOURCES})
        list(APPEND SCRIPT_HPPS ${GAME_SCRIPT_HEADERS})
        target_include_directories(game_macos PRIVATE ${GAME_SCRIPT_INCLUDES})
    endif()

    if(EXISTS "${BUILD_DIR}/assets")
        file(GLOB_RECURSE CURR_ASSETS "${BUILD_DIR}/assets/*")
        list(APPEND ASSET_FILES ${CURR_ASSETS})
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
    string(APPEND REGISTER_SCRIPT_LINES "    zzz::script::ScriptRegistry::Register<${QUALIFIED_NAME}>(\"${QUALIFIED_NAME}\");\n")
endforeach()

string(APPEND REGISTER_SCRIPTS_CONTENT "\nvoid RegisterAllScripts()\n{\n${REGISTER_SCRIPT_LINES}}\n")
set(GENERATED_REGISTER_CPP "${CMAKE_CURRENT_BINARY_DIR}/RegisterAllScripts.cpp")
file(WRITE "${GENERATED_REGISTER_CPP}" "${REGISTER_SCRIPTS_CONTENT}")

target_include_directories(game_macos PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/scene"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/userscripts"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/userscripts/base_script"
)

target_sources(game_macos PRIVATE
    ${SCRIPT_CPPS}
    ${GENERATED_REGISTER_CPP}
    ${ASSET_FILES}
)

if(ASSET_FILES)
    set_source_files_properties(${ASSET_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources/assets"
    )
endif()

target_link_libraries(game_macos
    PRIVATE
        engine_lib
        logger_lib
)

target_compile_features(game_macos PUBLIC cxx_std_23)

find_library(COCOA_LIBRARY Cocoa)
find_library(METAL_LIBRARY Metal)
find_library(METALKIT_LIBRARY MetalKit)
find_library(QUARTZCORE_LIBRARY QuartzCore)

target_link_libraries(game_macos
    PRIVATE
        ${COCOA_LIBRARY}
        ${METAL_LIBRARY}
        ${METALKIT_LIBRARY}
        ${QUARTZCORE_LIBRARY}
)

set_target_properties(game_macos PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER com.zzz.game
    MACOSX_BUNDLE_BUNDLE_NAME Zzz
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER com.zzz.game
    XCODE_SCHEME_ENVIRONMENT "OS_ACTIVITY_MODE=disable"
)

target_link_options(game_macos PRIVATE "-ObjC")
target_compile_definitions(game_macos PRIVATE Z_TARGET_NAME="game_macos")



