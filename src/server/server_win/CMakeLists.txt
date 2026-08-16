add_executable(server_win
    src/main.cpp
)

target_link_libraries(server_win PRIVATE server_lib core_lib logger_lib)

if(WIN32)
    target_precompile_headers(server_win PRIVATE
        <windows.h>
        <iostream>
        <vector>
        <string>
    )
endif()

