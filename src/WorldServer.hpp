#ifndef WORLD_SERVER_HPP
#define WORLD_SERVER_HPP

#include <map>
#include <string>

#include "Chunk.hpp"
#include "util.hpp"
#include "vec5.hpp"

class WorldServer {
   public:
    void Run(uint16 nPort);

   private:
    HSteamListenSocket listenSock;
    HSteamNetPollGroup pollGroup;
    ISteamNetworkingSockets *interface;

    struct Client {
        std::string name;
    };

    std::map<HSteamNetConnection, Client> clients;
    bool quit = false;
    std::string worldDir = "world";

    void SendStringToClient(HSteamNetConnection conn, const char *str);
    void SendStringToAllClients(const char *str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
    void PollIncomingMessages();
    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info);
    void generateChunk(ChunkLoc loc, Chunk &chunk);
    void saveChunk(ChunkLoc loc, const Chunk &chunk);
    void sendChunk(ChunkLoc loc, HSteamNetConnection connection);

    static WorldServer *callbackInstance;
    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *info);

    void PollConnectionStateChanges();
};

#endif // WORLD_SERVER_HPP
