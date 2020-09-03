#ifndef WORLD_CLIENT_HPP
#define WORLD_CLIENT_HPP

#include <queue>

#include "../common/util.hpp"
#include "../common/vec5.hpp"
#include "../common/Chunk.hpp"
#include "GeometryChunk.hpp"

class WorldClient {
   public:
    WorldClient(World &w);
    void Run(const SteamNetworkingIPAddr &serverAddr);
    void requestChunk(ChunkLoc loc);
    void pollEvents();

    std::queue<std::pair<ChunkLoc, GeometryChunk> > requestedChunks;

   private:
    HSteamNetConnection connection;
    ISteamNetworkingSockets *interface;
    World &world;
    bool quit = false;

    void PollIncomingMessages();
    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info);
    static WorldClient *callbackInstance;
    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *info);
    void PollConnectionStateChanges();
};

#endif // WORLD_CLIENT_HPP
