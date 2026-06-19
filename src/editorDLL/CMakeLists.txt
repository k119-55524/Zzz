add_library(editorDLL SHARED)

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


