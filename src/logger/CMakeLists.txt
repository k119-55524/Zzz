macro(setup_logger_target TARGET_NAME CORE_TARGET_NAME IS_EDITOR)
    add_library(${TARGET_NAME})

    target_precompile_headers(${TARGET_NAME} PRIVATE
        pch/pch.h
    )

    target_include_directories(${TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_sources(${TARGET_NAME} PRIVATE
        logger.h
        logger.cpp
        LoggerIncludes.h
        private/log_entry.h
        private/log_entry.cpp
        private/IBroadcaster.h
        private/ConsoleBroadcaster.h
        private/NetworkBroadcaster.h
        private/NetworkBroadcaster.cpp
    )

    target_compile_features(${TARGET_NAME} PUBLIC cxx_std_23)
    target_compile_definitions(${TARGET_NAME} PRIVATE Z_TARGET_NAME="${TARGET_NAME}")
    set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

    target_link_libraries(${TARGET_NAME} PUBLIC ${CORE_TARGET_NAME})

    if(WIN32)
        target_link_libraries(${TARGET_NAME} PRIVATE User32 Ws2_32)
    endif()

    if(ANDROID)
        target_link_libraries(${TARGET_NAME} PRIVATE log)
    endif()

    if(${IS_EDITOR})
        # logger_lib_editor должен совпадать по define'ам с editor-вариантами core/engine,
        # иначе в editor_dll.dll попадут разные варианты экспортируемых символов.
        target_compile_definitions(${TARGET_NAME} PUBLIC Z_EDITOR=1 EDITORDLL_EXPORTS)
    endif()

    export_to_editor_sdk(${TARGET_NAME} "${CMAKE_CURRENT_SOURCE_DIR}" "logger")
endmacro()

setup_logger_target(logger_lib core_lib FALSE)
setup_logger_target(logger_lib_editor core_lib_editor TRUE)
