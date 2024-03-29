#include "client/App.hpp"
#include "client/WorldClient.hpp"
#include "server/WorldServer.hpp"
#include "common/FlatWorldGenerator.hpp"

#include <thread>

const uint16 DEFAULT_SERVER_PORT = 27020;

void printUsageAndExit(int rc = 1) {
    fflush(stderr);
    printf(
        R"usage(Usage:
    5d client SERVER_ADDR
    5d server [--port PORT]
)usage");
    fflush(stdout);
    exit(rc);
}

int main(int argc, const char *argv[]) {
    if (argc == 1) {
        App app;
        FlatWorldGenerator gen;
        app.world->generator = &gen;
        app.run();
        return 0;
    } else if (argc == 2 && strncmp(argv[1], "--help", 7)) {
        printUsageAndExit();
    }
    bool bServer = false;
    bool bClient = false;
    int nPort = DEFAULT_SERVER_PORT;
    SteamNetworkingIPAddr addrServer;
    addrServer.Clear();

    for (int i = 1; i < argc; ++i) {
        if (!bClient && !bServer) {
            if (!strcmp(argv[i], "client")) {
                bClient = true;
                continue;
            }
            if (!strcmp(argv[i], "server")) {
                bServer = true;
                continue;
            }
        }
        if (!strcmp(argv[i], "--port")) {
            ++i;
            if (i >= argc) {
                printUsageAndExit();
            }
            nPort = atoi(argv[i]);
            if (nPort <= 0 || nPort > 65535) {
                FatalError("Invalid port %d", nPort);
            }
            continue;
        }

        // Anything else, must be server address to connect to
        if (bClient && addrServer.IsIPv6AllZeros()) {
            if (!addrServer.ParseString(argv[i])) {
                FatalError("Invalid server address '%s'", argv[i]);
            }
            if (addrServer.m_port == 0) {
                addrServer.m_port = DEFAULT_SERVER_PORT;
            }
            continue;
        }

        printUsageAndExit();
    }

    if (bClient == bServer || (bClient && addrServer.IsIPv6AllZeros())) {
        printUsageAndExit();
    }

    InitSteamDatagramConnectionSockets();

    if (bClient) {
        App app;
        WorldClient client(*app.world);
        app.world->client = &client;
        client.Run(addrServer);
        app.run();
    } else {
        WorldServer server;
        FlatWorldGenerator gen;
        server.generator = &gen;
        server.Run((uint16)nPort);
    }

    ShutdownSteamDatagramConnectionSockets();
}
