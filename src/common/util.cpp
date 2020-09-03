#include "util.hpp"

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>

#include <thread>

SteamNetworkingMicroseconds logTimeZero;

void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg) {
    SteamNetworkingMicroseconds time = SteamNetworkingUtils()->GetLocalTimestamp() - logTimeZero;
    printf("%10.6f %s\n", time * 1e-6, pszMsg);
    fflush(stdout);
    if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
        fflush(stdout);
        fflush(stderr);
        kill(getpid(), SIGKILL);
    }
}

void FatalError(const char *fmt, ...) {
    char text[2048];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    char *nl = strchr(text, '\0') - 1;
    if (nl >= text && *nl == '\n')
        *nl = '\0';
    DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
}

void Printf(const char *fmt, ...) {
    char text[2048];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(text, fmt, ap);
    va_end(ap);
    char *nl = strchr(text, '\0') - 1;
    if (nl >= text && *nl == '\n')
        *nl = '\0';
    DebugOutput(k_ESteamNetworkingSocketsDebugOutputType_Msg, text);
}

void InitSteamDatagramConnectionSockets() {
    SteamDatagramErrMsg errMsg;
    if (!GameNetworkingSockets_Init(nullptr, errMsg)) {
        FatalError("GameNetworkingSockets_Init failed.  %s", errMsg);
    }

    logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();
    SteamNetworkingUtils()->SetDebugOutputFunction(k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput);
}

void ShutdownSteamDatagramConnectionSockets() {
    // Give connections time to finish up.  This is an application layer protocol
    // here, it's not TCP.  Note that if you have an application and you need to be
    // more sure about cleanup, you won't be able to do this.  You will need to send
    // a message and then either wait for the peer to close the connection, or
    // you can pool the connection to see if any reliable data is pending.
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    GameNetworkingSockets_Kill();
}
