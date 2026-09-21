add_library(server_lib STATIC
    src/server_engine.cpp
)

target_include_directories(server_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(server_lib PUBLIC core_lib)

if(WIN32)
    target_precompile_headers(server_lib PRIVATE
        <windows.h>
        <winsock2.h>
        <ws2tcpip.h>
        <vector>
        <string>
        <memory>
        <expected>
    )
endif()

