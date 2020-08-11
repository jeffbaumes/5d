#include "WorldClient.hpp"

#include <thread>

WorldClient *WorldClient::callbackInstance;

void WorldClient::Run(const SteamNetworkingIPAddr &serverAddr) {
    // Select instance to use.  For now we'll always use the default.
    interface = SteamNetworkingSockets();

    // Start connecting
    char addr[SteamNetworkingIPAddr::k_cchMaxString];
    serverAddr.ToString(addr, sizeof(addr), true);
    Printf("Connecting to chat server at %s", addr);
    SteamNetworkingConfigValue_t opt;
    opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void *)SteamNetConnectionStatusChangedCallback);
    connection = interface->ConnectByIPAddress(serverAddr, 1, &opt);
    if (connection == k_HSteamNetConnection_Invalid) {
        FatalError("Failed to create connection");
    }

    while (!quit) {
        PollIncomingMessages();
        PollConnectionStateChanges();
        // PollLocalUserInput();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void WorldClient::PollIncomingMessages() {
    while (!quit) {
        ISteamNetworkingMessage *incomingMsg = nullptr;
        int numMsgs = interface->ReceiveMessagesOnConnection(connection, &incomingMsg, 1);
        if (numMsgs == 0) {
            break;
        }
        if (numMsgs < 0) {
            FatalError("Error checking for messages");
        }

        // Just echo anything we get from the server
        fwrite(incomingMsg->m_pData, 1, incomingMsg->m_cbSize, stdout);
        fputc('\n', stdout);

        // We don't need this anymore.
        incomingMsg->Release();
    }
}

void WorldClient::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info) {
    assert(info->m_hConn == connection || connection == k_HSteamNetConnection_Invalid);

    // What's the state of the connection?
    switch (info->m_info.m_eState) {
        case k_ESteamNetworkingConnectionState_None:
            // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
            break;

        case k_ESteamNetworkingConnectionState_ClosedByPeer:
        case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
            quit = true;

            // Print an appropriate message
            if (info->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
                // Note: we could distinguish between a timeout, a rejected connection,
                // or some other transport problem.
                Printf("Connection to server failed.  (%s)", info->m_info.m_szEndDebug);
            } else if (info->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
                Printf("Lost connection to server.  (%s)", info->m_info.m_szEndDebug);
            } else {
                // NOTE: We could check the reason code for a normal disconnection
                Printf("Disconnected from server.  (%s)", info->m_info.m_szEndDebug);
            }

            // Clean up the connection.  This is important!
            // The connection is "closed" in the network sense, but
            // it has not been destroyed.  We must close it on our end, too
            // to finish up.  The reason information do not matter in this case,
            // and we cannot linger because it's already closed on the other end,
            // so we just pass 0's.
            interface->CloseConnection(info->m_hConn, 0, nullptr, false);
            connection = k_HSteamNetConnection_Invalid;
            break;
        }

        case k_ESteamNetworkingConnectionState_Connecting:
            // We will get this callback when we start connecting.
            // We can ignore this.
            break;

        case k_ESteamNetworkingConnectionState_Connected:
            Printf("Connected to server OK");
            break;

        default:
            // Silences -Wswitch
            break;
    }
}

void WorldClient::SteamNetConnectionStatusChangedCallback(SteamNetConnectionStatusChangedCallback_t *info) {
    callbackInstance->OnSteamNetConnectionStatusChanged(info);
}

void WorldClient::PollConnectionStateChanges() {
    callbackInstance = this;
    interface->RunCallbacks();
}
