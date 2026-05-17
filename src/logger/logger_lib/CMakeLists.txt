add_library(logger_lib)

target_precompile_headers(logger_lib PRIVATE
    pch/pch.h
)

target_include_directories(logger_lib PRIVATE
    pch
)

target_sources(logger_lib
    PUBLIC
        FILE_SET CXX_MODULES FILES
            modules/logger.ixx
)

if(ZZZ_ENABLE_LOGGER)

    target_sources(logger_lib
        PRIVATE
            src/logger.cpp
    )

    target_compile_definitions(logger_lib PUBLIC
        ZZZ_ENABLE_LOGGER=1
    )

else()

    target_compile_definitions(logger_lib PUBLIC
        ZZZ_ENABLE_LOGGER=0
	)

endif()

target_compile_features(logger_lib PUBLIC cxx_std_23)