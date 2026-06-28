add_library(editorDLL SHARED)

# editorDLL is also built natively via editorDLL.vcxproj in src/tools/Tools.sln, which writes
# straight into the shared bin/$(Configuration) used by Tools.sln/editor.exe. Keep this CMake
# build's output in a separate folder so it can never silently overwrite (or be overwritten by)
# that MSBuild output and desync the debugger's symbols from the loaded module. editor.csproj's
# standalone fallback build copies the result into bin/$(Configuration) explicitly when needed.
set_target_properties(editorDLL PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin_standalone"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin_standalone"
    PDB_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin_standalone"
)

target_sources(editorDLL PRIVATE
    Core/editorDLL.h
    Core/editorDLL.cpp
    EngineWrapper/EditorEngine.h
    EngineWrapper/EditorEngine.cpp
)

target_include_directories(editorDLL PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(editorDLL
    PRIVATE
        engine_lib_editor
)
target_compile_definitions(editorDLL PRIVATE Z_TARGET_NAME="editorDLL")


