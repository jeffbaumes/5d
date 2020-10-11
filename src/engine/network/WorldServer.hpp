#pragma once

class ChunkRequestHandler;

class WorldServer {
public:
    WorldServer(int port, ChunkRequestHandler &handler);
    void run();
};
