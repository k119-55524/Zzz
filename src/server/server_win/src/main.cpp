#include "server/server_engine.h"
#include <iostream>

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    std::cout << "[Zzz Server] Starting Standalone Dedicated Server..." << std::endl;

    zzz::server::ServerEngine engine;
    if (engine.Initialize()) {
        std::cout << "[Zzz Server] Running. Press ENTER to stop..." << std::endl;
        std::cin.get();
        engine.Shutdown();
    }

    return 0;
}
