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
    private/IBroadcaster.h
)

target_compile_features(logger_lib PUBLIC cxx_std_23)

target_link_libraries(logger_lib PUBLIC common_lib)

if(ZZZ_ENABLE_LOGGER)
    target_compile_definitions(logger_lib PUBLIC ZZZ_ENABLE_LOGGER=1)
endif()

if(WIN32)
    target_link_libraries(logger_lib PRIVATE User32)
endif()

if(ANDROID)
    target_link_libraries(logger_lib PRIVATE log)
endif()
