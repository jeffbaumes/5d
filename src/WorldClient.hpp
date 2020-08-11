#ifndef WORLD_CLIENT_HPP
#define WORLD_CLIENT_HPP

#include "util.hpp"

class WorldClient {
   public:
    void Run(const SteamNetworkingIPAddr &serverAddr);

   private:
    HSteamNetConnection connection;
    ISteamNetworkingSockets *interface;
    bool quit = false;

    void PollIncomingMessages();
    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info);
    static WorldClient *callbackInstance;
    static void SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *info);
    void PollConnectionStateChanges();
};

#endif // WORLD_CLIENT_HPP
