add_library(logger_lib)

target_precompile_headers(logger_lib PRIVATE
    pch/pch.h
)

target_include_directories(logger_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(logger_lib PRIVATE
    pch/pch.h
    pch/pch.cpp
    logger.h
    logger.cpp
    header.h
    log_entry.h
    log_entry.cpp
    private/IBroadcaster.h
    private/ConsoleBroadcaster.h
    private/NetworkBroadcaster.h
    private/NetworkBroadcaster.cpp
)

target_compile_features(logger_lib PUBLIC cxx_std_23)
target_compile_definitions(logger_lib PRIVATE Z_TARGET_NAME="logger_lib")
set_source_files_properties(pch/pch.cpp PROPERTIES SKIP_PRECOMPILE_HEADERS ON)

target_link_libraries(logger_lib PUBLIC common_lib)

if(WIN32)
    target_link_libraries(logger_lib PRIVATE User32 Ws2_32)
endif()

if(ANDROID)
    target_link_libraries(logger_lib PRIVATE log)
endif()

export_to_editor_sdk(logger_lib "${CMAKE_CURRENT_SOURCE_DIR}" "logger")
