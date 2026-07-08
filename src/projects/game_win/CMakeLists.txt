add_executable(game_win WIN32)

set(Z_GAME_PROJECT_DIR "" CACHE PATH "Path to the active game project")

set(SCRIPT_HPPS "")
set(SCRIPT_CPPS "")

if(Z_GAME_PROJECT_DIR)
    file(GLOB_RECURSE SCRIPT_HPPS "${Z_GAME_PROJECT_DIR}/Assets/*.hpp")
    file(GLOB_RECURSE SCRIPT_CPPS "${Z_GAME_PROJECT_DIR}/Assets/*.cpp")
endif()

set(REGISTER_SCRIPTS_CONTENT "// RegisterAllScripts.cpp — генерируется автоматически CMake\n#include <ScriptRegistry.h>\n\n")

# Собираем список имен классов и инклуды
set(REGISTER_SCRIPT_LINES "")
foreach(HPP_FILE ${SCRIPT_HPPS})
    if(Z_GAME_PROJECT_DIR)
        file(RELATIVE_PATH REL_PATH "${Z_GAME_PROJECT_DIR}/Assets" "${HPP_FILE}")
    endif()
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

    string(APPEND REGISTER_SCRIPTS_CONTENT "#include \"${REL_PATH}\"
")
    string(APPEND REGISTER_SCRIPT_LINES "    zzz::script::ScriptRegistry::Register<${QUALIFIED_NAME}>(\"${QUALIFIED_NAME}\");
")
endforeach()

string(APPEND REGISTER_SCRIPTS_CONTENT "
void RegisterAllScripts()
{
")
string(APPEND REGISTER_SCRIPTS_CONTENT "${REGISTER_SCRIPT_LINES}")
string(APPEND REGISTER_SCRIPTS_CONTENT "}
")

set(GENERATED_REGISTER_CPP "${CMAKE_CURRENT_BINARY_DIR}/RegisterAllScripts.cpp")
file(WRITE "${GENERATED_REGISTER_CPP}" "${REGISTER_SCRIPTS_CONTENT}")

target_precompile_headers(game_win PRIVATE
    pch/pch.h
)

target_include_directories(game_win PRIVATE
    pch
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/scene"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/scene/scripts"
    "${CMAKE_SOURCE_DIR}/src/engine/public/core/scene/scripts/base_script"
)

if(Z_GAME_PROJECT_DIR)
    target_include_directories(game_win PRIVATE "${Z_GAME_PROJECT_DIR}/Assets")
endif()

target_sources(game_win PRIVATE
    pch/pch.h
    pch/pch.cpp
    main.h
    main.cpp
    resource/app.rc
    ${SCRIPT_CPPS}
    ${GENERATED_REGISTER_CPP}
)

target_link_libraries(game_win PRIVATE
    engine_lib
    logger_lib
)

target_compile_features(game_win PUBLIC cxx_std_23)
target_compile_definitions(game_win PRIVATE Z_TARGET_NAME="game_win")
set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
