add_library(editor_dll SHARED)

# editor_dll is also built natively via editor_dll.vcxproj in src/tools/Tools.sln, which writes
# straight into the shared bin/$(Configuration) used by Tools.sln/editor.exe. Keep this CMake
# build's output in a separate folder so it can never silently overwrite (or be overwritten by)
# that MSBuild output and desync the debugger's symbols from the loaded module. editor.csproj's
# standalone fallback build copies the result into bin/$(Configuration) explicitly when needed.
set_target_properties(editor_dll PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin_standalone"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin_standalone"
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin_standalone"
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_sources(editor_dll PRIVATE
    core/editor_dll.h
    core/editor_dll.cpp
    engine_wrapper/EditorEngine.h
    engine_wrapper/EditorEngine.cpp
)

target_include_directories(editor_dll PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(editor_dll
    PRIVATE
        engine_lib_editor
)
target_compile_definitions(editor_dll PRIVATE Z_TARGET_NAME="editor_dll")
