#include "WorldServer.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

#include "../common/Chunk.hpp"
#include "../common/vec5.hpp"

WorldServer *WorldServer::callbackInstance;

void WorldServer::Run(uint16 nPort) {
    interface = SteamNetworkingSockets();

    // Start listening
    SteamNetworkingIPAddr serverLocalAddr;
    serverLocalAddr.Clear();
    serverLocalAddr.m_port = nPort;
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *)SteamNetConnectionStatusChangedCallback);
    listenSock = interface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
    if (listenSock == k_HSteamListenSocket_Invalid) {
        FatalError("Failed to listen on port %d", nPort);
    }
    pollGroup = interface->CreatePollGroup();
    if (pollGroup == k_HSteamNetPollGroup_Invalid) {
        FatalError("Failed to listen on port %d", nPort);
    }
    Printf("Server listening on port %d\n", nPort);

    while (!quit) {
        PollIncomingMessages();
        PollConnectionStateChanges();
    }

    // Close all the connections
    Printf("Closing connections...\n");
    for (auto it : clients) {
        // Send them one more goodbye message.  Note that we also have the
        // connection close reason as a place to send final data.  However,
        // that's usually best left for more diagnostic/debug text not actual
        // protocol strings.
        SendStringToClient(it.first, "Server is shutting down.  Goodbye.");

        // Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
        // to flush this out and close gracefully.
        interface->CloseConnection(it.first, 0, "Server Shutdown", true);
    }
    clients.clear();

    interface->CloseListenSocket(listenSock);
    listenSock = k_HSteamListenSocket_Invalid;

    interface->DestroyPollGroup(pollGroup);
    pollGroup = k_HSteamNetPollGroup_Invalid;
}

void WorldServer::SendStringToClient(HSteamNetConnection conn, const char *str) {
    interface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
}

void WorldServer::SendStringToAllClients(const char *str, HSteamNetConnection except) {
    for (auto &c : clients) {
        if (c.first != except) {
            SendStringToClient(c.first, str);
        }
    }
}

void WorldServer::generateChunk(ChunkLoc loc, Chunk &chunk) {
    for (int x = 0; x < CHUNK_SIZE_XZUV; x += 1) {
        for (int y = 0; y < CHUNK_SIZE_Y; y += 1) {
            for (int z = 0; z < CHUNK_SIZE_XZUV; z += 1) {
                for (int u = 0; u < CHUNK_SIZE_XZUV; u += 1) {
                    for (int v = 0; v < CHUNK_SIZE_XZUV; v += 1) {
                        int material = 0;
                        if (y == 3) {
                            material = 3;
                        }
                        if (y == 2) {
                            material = 2;
                        }
                        if (y == 1) {
                            material = 1;
                        }
                        if (material > 0) {
                            chunk[{x, y, z, u, v}] = material;
                        }
                    }
                }
            }
        }
    }
}

void WorldServer::saveChunk(ChunkLoc loc, const Chunk &chunk) {
    Printf("Saving a chunk (not really)");
}

void WorldServer::sendChunk(ChunkLoc loc, HSteamNetConnection connection) {
    Chunk chunk;
    std::string filename = std::to_string(loc.x) + "_" + std::to_string(loc.y) + "_" + std::to_string(loc.z) + "_" + std::to_string(loc.u) + "_" + std::to_string(loc.v);
    std::ifstream file(worldDir + "/" + filename, std::ios::out | std::ios::binary);
    file.read(reinterpret_cast<char *>(chunk.cells.data()), sizeof(int) * CHUNK_SIZE);
    if (!file.good()) {
        generateChunk(loc, chunk);
        saveChunk(loc, chunk);
    }

    // Send the chunk
    Printf("Sending chunk");
    loc.print();
    std::stringstream msg;
    msg.put(0);
    msg.write(reinterpret_cast<char *>(&loc), sizeof(ChunkLoc));
    msg.write(reinterpret_cast<char *>(chunk.cells.data()), sizeof(int) * CHUNK_SIZE);
    interface->SendMessageToConnection(connection, msg.str().c_str(), msg.str().length(), k_nSteamNetworkingSend_Reliable, nullptr);
}

void WorldServer::PollIncomingMessages() {
    char temp[1024];

    while (!quit) {
        ISteamNetworkingMessage *incomingMsg = nullptr;
        int numMsgs = interface->ReceiveMessagesOnPollGroup(pollGroup, &incomingMsg, 1);
        if (numMsgs == 0) {
            break;
        }

        if (numMsgs < 0) {
            FatalError("Error checking for messages");
        }
        assert(numMsgs == 1 && incomingMsg);
        auto itClient = clients.find(incomingMsg->m_conn);
        assert(itClient != clients.end());

        auto conn = incomingMsg->m_conn;
        auto msg = static_cast<char *>(incomingMsg->m_pData);
        auto len = incomingMsg->m_cbSize;

        // Check message and perform action
        size_t ind = 0;
        if (msg[0] == 0) {
            ind += 1;
            auto locPtr = reinterpret_cast<int *>(&msg[ind]);
            ChunkLoc loc(locPtr[0], locPtr[1], locPtr[2], locPtr[3], locPtr[4]);
            incomingMsg->Release();
            loc.print();
            sendChunk(loc, conn);

            // If we don't sleep a bit the UDP channel seems to get saturated and not
            // all chunks make it to the client.
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        } else {
            incomingMsg->Release();
        }
    }
}

void WorldServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info) {
    char temp[1024];

    // What's the state of the connection?
    switch (info->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            // Ignore if they were not previously connected.  (If they disconnected
            // before we accepted the connection.)
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
                // Locate the client.  Note that it should have been found, because this
                // is the only codepath where we remove clients (except on shutdown),
                // and connection change callbacks are dispatched in queue order.
                auto itClient = clients.find(info->m_hConn);
                assert(itClient != clients.end());

                // Select appropriate log messages
                const char *debugLogAction;
                if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
                    debugLogAction = "problem detected locally";
                    sprintf(temp, "Alas, %s hath fallen into shadow.  (%s)", itClient->second.name.c_str(), info->m_info.m_szEndDebug);
                } else {
                    // Note that here we could check the reason code to see if
                    // it was a "usual" connection or an "unusual" one.
                    debugLogAction = "closed by peer";
                    sprintf(temp, "%s hath departed", itClient->second.name.c_str());
                }

                // Spew something to our own log.  Note that because we put their nick
                // as the connection description, it will show up, along with their
                // transport-specific data (e.g. their IP address)
                Printf("Connection %s %s, reason %d: %s\n",
                       info->m_info.m_szConnectionDescription,
                       debugLogAction,
                       info->m_info.m_eEndReason,
                       info->m_info.m_szEndDebug);

                clients.erase(itClient);

                // Send a message so everybody else knows what happened
                SendStringToAllClients(temp);
            } else {
                assert(info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
            }

            // Clean up the connection.  This is important!
            // The connection is "closed" in the network sense, but
            // it has not been destroyed.  We must close it on our end, too
            // to finish up.  The reason information do not matter in this case,
            // and we cannot linger because it's already closed on the other end,
            // so we just pass 0's.
            interface->CloseConnection(info->m_hConn, 0, nullptr, false);
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting: {
            // This must be a new connection
            assert(clients.find(info->m_hConn) == clients.end());

            Printf("Connection request from %s", info->m_info.m_szConnectionDescription);

            // A client is attempting to connect
            // Try to accept the connection.
            if (interface->AcceptConnection(info->m_hConn) != k_EResultOK) {
                // This could fail.  If the remote host tried to connect, but then
                // disconnected, the connection may already be half closed.  Just
                // destroy whatever we have on our side.
                interface->CloseConnection(info->m_hConn, 0, nullptr, false);
                Printf("Can't accept connection.  (It was already closed?)");
                break;
            }

            // Assign the poll group
            if (!interface->SetConnectionPollGroup(info->m_hConn, pollGroup)) {
                interface->CloseConnection(info->m_hConn, 0, nullptr, false);
                Printf("Failed to set poll group?");
                break;
            }

            SendStringToClient(info->m_hConn, "You are connected to the server.");
            SendStringToAllClients("Someone connected to the server.", info->m_hConn);

            clients[info->m_hConn] = {"unknown"};
            break;
        }

        case k_ESteamNetworkingConnectionState_Connected:
            // We will get a callback immediately after accepting the connection.
            // Since we are the server, we can ignore this, it's not news to us.
            break;

        default:
            // Silences -Wswitch
            break;
    }
}

void WorldServer::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *info) {
    callbackInstance->OnSteamNetConnectionStatusChanged(info);
}

void WorldServer::PollConnectionStateChanges() {
    callbackInstance = this;
    interface->RunCallbacks();
}
