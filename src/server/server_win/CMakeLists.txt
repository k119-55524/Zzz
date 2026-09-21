add_executable(server_win
    src/main.cpp
    resource/server_win.rc
)

target_link_libraries(server_win PRIVATE server_lib core_lib)

if(WIN32)
    target_precompile_headers(server_win PRIVATE
        <windows.h>
        <iostream>
        <vector>
        <string>
    )
endif()
