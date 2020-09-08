#include <iostream>
#include <memory>
#include <string>

#include "engine/core/FlatChunkGenerator.hpp"
#include "engine/core/LocalChunkHandler.hpp"
#include "engine/core/World.hpp"
// #include "engine/network/WorldClient.hpp"
// #include "engine/render/WorldView.hpp"

int main(int argc, char *argv[]) {
    World world;
    if (argc == 3 && std::string(argv[1]) == "--server") {
        std::cout << "Not implemented" << std::endl;
        // auto client = std::make_shared<WorldClient>(argv[2]);
        // world.addWorldListener(client);
        // world.addWorldTask(client);
        // world.setChunkRequestHandler(client);
        // world.run();
    } else if (argc == 2) {
        auto gen = std::make_unique<FlatChunkGenerator>();
        auto handler = std::make_shared<LocalChunkHandler>(argv[1], std::move(gen));
        world.addWorldListener(handler);
        world.setChunkRequestHandler(handler);
        world.run();
    } else {
        std::cout << "Usage:" << std::endl;
        std::cout << "  5d --server [server:port]" << std::endl;
        std::cout << "  5d [world_dir]" << std::endl;
    }
    return 0;
}
