add_library(editorDLL SHARED)

target_sources(editorDLL PRIVATE
    editorDLL.h
    editorDLL.cpp
)

target_include_directories(editorDLL PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(editorDLL
    PRIVATE
        engine_lib_editor
)

# Копируем собранную DLL в папку C# WPF редактора, чтобы она копировалась при его сборке
add_custom_command(TARGET editorDLL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:editorDLL> "${CMAKE_SOURCE_DIR}/src/editor/"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_PDB_FILE:editorDLL> "${CMAKE_SOURCE_DIR}/src/editor/"
    COMMENT "Copying editorDLL to C# project folder"
)

if(MSVC)
    # Копируем PDB файл для отладки
    add_custom_command(TARGET editorDLL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_PDB_FILE:editorDLL> "${CMAKE_SOURCE_DIR}/src/editor/editorDLL.pdb"
    )
endif()
