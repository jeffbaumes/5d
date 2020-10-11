#include <iostream>
#include <string>

#include "engine/core/FlatChunkGenerator.hpp"
#include "engine/core/LocalChunkHandler.hpp"
#include "engine/network/WorldServer.hpp"

int main(int argc, char *argv[]) {
    if (argc == 3) {
        FlatChunkGenerator gen;
        LocalChunkHandler handler(argv[1], gen);
        WorldServer server(atoi(argv[2]), handler);
        server.run();
    } else {
        std::cout << "Usage:" << std::endl;
        std::cout << "  5d-server [world_dir] [port]" << std::endl;
    }
    return 0;
}
