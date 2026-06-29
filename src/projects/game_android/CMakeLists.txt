find_package(game-activity REQUIRED CONFIG)

add_library(game_android SHARED)

set(Z_GAME_PROJECT_DIR "${CMAKE_SOURCE_DIR}/src/editor_projects/super_game" CACHE PATH "Path to the active game project")

# Находим все файлы скриптов
file(GLOB_RECURSIVE SCRIPT_HPPS "${Z_GAME_PROJECT_DIR}/Assets/*.hpp")
file(GLOB_RECURSIVE SCRIPT_CPPS "${Z_GAME_PROJECT_DIR}/Assets/*.cpp")

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

target_precompile_headers(game_android PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:pch/pch.h>"
)

target_include_directories(game_android PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine/private/core"
    "${CMAKE_SOURCE_DIR}/src/engine/private/core/scene"
    "${CMAKE_SOURCE_DIR}/src/engine/private/core/scene/scripts"
    "${CMAKE_SOURCE_DIR}/src/engine/private/core/scene/scripts/base_script"
    "${Z_GAME_PROJECT_DIR}/Assets"
)

target_sources(game_android PRIVATE
    pch/pch.h
    pch/pch.cpp
    src/android_main.h
    src/android_main.cpp
    ${SCRIPT_CPPS}
    ${GENERATED_REGISTER_CPP}
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
