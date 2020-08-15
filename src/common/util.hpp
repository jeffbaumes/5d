#ifndef UTIL_HPP
#define UTIL_HPP

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

void DebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg);
void FatalError(const char *fmt, ...);
void Printf(const char *fmt, ...);

void InitSteamDatagramConnectionSockets();
void ShutdownSteamDatagramConnectionSockets();

#endif // UTIL_HPP
