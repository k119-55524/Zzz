add_executable(game_ios
    MACOSX_BUNDLE

    main.mm
)

set(Z_GAME_PROJECT_DIR "${CMAKE_SOURCE_DIR}/src/editor_projects/super_game" CACHE PATH "Path to the active game project")

# Находим все файлы скриптов
file(GLOB_RECURSE SCRIPT_HPPS "${Z_GAME_PROJECT_DIR}/Assets/*.hpp")
file(GLOB_RECURSE SCRIPT_CPPS "${Z_GAME_PROJECT_DIR}/Assets/*.cpp")

set(REGISTER_SCRIPTS_CONTENT "// RegisterAllScripts.cpp — generated automatically by CMake\n#include <ScriptRegistry.h>\n\n")

# Собираем список имен классов и инклуды
set(SCRIPT_NAMES "")
foreach(HPP_FILE ${SCRIPT_HPPS})
    file(RELATIVE_PATH REL_PATH "${Z_GAME_PROJECT_DIR}/Assets" "${HPP_FILE}")
    get_filename_component(CLASS_NAME "${HPP_FILE}" NAME_WE)
    string(APPEND REGISTER_SCRIPTS_CONTENT "#include \"${REL_PATH}\"\n")
    list(APPEND SCRIPT_NAMES "${CLASS_NAME}")
endforeach()

string(APPEND REGISTER_SCRIPTS_CONTENT "\nvoid RegisterAllScripts()\n{\n")
foreach(CLASS_NAME ${SCRIPT_NAMES})
    string(APPEND REGISTER_SCRIPTS_CONTENT "    zzz::script::ScriptRegistry::Register<${CLASS_NAME}>(\"${CLASS_NAME}\");\n")
endforeach()
string(APPEND REGISTER_SCRIPTS_CONTENT "}\n")

set(GENERATED_REGISTER_CPP "${CMAKE_CURRENT_BINARY_DIR}/RegisterAllScripts.cpp")
file(WRITE "${GENERATED_REGISTER_CPP}" "${REGISTER_SCRIPTS_CONTENT}")

target_include_directories(game_ios PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine/private/core"
    "${CMAKE_SOURCE_DIR}/src/engine/private/core/scene"
    "${CMAKE_SOURCE_DIR}/src/engine/private/core/scene/scripts"
    "${CMAKE_SOURCE_DIR}/src/engine/private/core/scene/scripts/base_script"
    "${Z_GAME_PROJECT_DIR}/Assets"
)

target_sources(game_ios PRIVATE
    ${SCRIPT_CPPS}
    ${GENERATED_REGISTER_CPP}
)

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
