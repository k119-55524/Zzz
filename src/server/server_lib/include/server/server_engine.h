#pragma once

namespace zzz::server {

class ServerEngine {
public:
    ServerEngine();
    ~ServerEngine();

    bool Initialize();
    void Shutdown();
};

} // namespace zzz::server
