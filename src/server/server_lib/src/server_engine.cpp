#include "server/server_engine.h"
#include <iostream>

namespace zzz::server {

ServerEngine::ServerEngine() = default;
ServerEngine::~ServerEngine() = default;

bool ServerEngine::Initialize() {
    std::cout << "[ServerEngine] Initialized." << std::endl;
    return true;
}

void ServerEngine::Shutdown() {
    std::cout << "[ServerEngine] Shutdown." << std::endl;
}

} // namespace zzz::server
