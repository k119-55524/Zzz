add_executable(game_ios
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
set(ASSET_FILES "")

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
        target_include_directories(game_ios PRIVATE ${GAME_SCRIPT_INCLUDES})
    endif()

    if(EXISTS "${BUILD_DIR}/assets")
        file(GLOB_RECURSE CURR_ASSETS "${BUILD_DIR}/assets/*")
        list(APPEND ASSET_FILES ${CURR_ASSETS})
    endif()
endforeach()

set(REGISTER_SCRIPTS_CONTENT "// RegisterAllScripts.cpp — генерируется автоматически CMake\n#include <core/Core.h>\n#include <ScriptRegistry.h>\n\n")
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
    string(APPEND REGISTER_SCRIPT_LINES "    registrar.Register<${QUALIFIED_NAME}>(\"${QUALIFIED_NAME}\");\n")
endforeach()

string(APPEND REGISTER_SCRIPTS_CONTENT "\nvoid RegisterAllScripts(zzz::core::ScriptRegistrar& registrar)\n{\n${REGISTER_SCRIPT_LINES}}\n")
set(GENERATED_REGISTER_CPP "${CMAKE_CURRENT_BINARY_DIR}/RegisterAllScripts.cpp")
file(WRITE "${GENERATED_REGISTER_CPP}" "${REGISTER_SCRIPTS_CONTENT}")

target_include_directories(game_ios PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine/scene"
    "${CMAKE_SOURCE_DIR}/src/core/userscripts"
    "${CMAKE_SOURCE_DIR}/src/core/userscripts/base_script"
)

target_sources(game_ios PRIVATE
    ${SCRIPT_CPPS}
    ${ASSET_FILES}
)

if(ASSET_FILES)
    set_source_files_properties(${ASSET_FILES} PROPERTIES
        MACOSX_PACKAGE_LOCATION "assets"
    )
endif()

target_link_libraries(game_ios PRIVATE
    engine_lib
    logger_lib
)

target_link_libraries(game_ios PRIVATE
    "-framework UIKit"
    "-framework Foundation"
    "-framework Metal"
    "-framework MetalKit"
    "-framework QuartzCore"
)

set_target_properties(game_ios PROPERTIES
    MACOSX_BUNDLE YES

    MACOSX_BUNDLE_GUI_IDENTIFIER com.dzzzz.game
    MACOSX_BUNDLE_BUNDLE_NAME Zzz
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist

    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER com.dzzzz.game

    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 17.0

    XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION 1
    XCODE_ATTRIBUTE_MARKETING_VERSION 1.0

    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"

    XCODE_SCHEME_ENVIRONMENT "OS_ACTIVITY_MODE=disable"
)

target_link_options(game_ios PRIVATE "-ObjC")
target_compile_definitions(game_ios PRIVATE Z_TARGET_NAME="game_ios")
target_compile_features(game_ios PUBLIC cxx_std_23)



