#include "config.h"
#include "WebSocket/WebSocket.h"
#include <Net/assets/manager/logmanager.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore_static.lib")
#pragma comment(lib, "NetWebSocket_static.lib")
#endif

int main()
{
	NET_INITIALIZE(Net::ENABLE_LOGGING);

	WebSocket ws;
	ws.SetOption<char*>({ NET_OPT_NAME, (char*)SANDBOX_SERVERNAME });
	ws.SetOption<u_short>({ NET_OPT_PORT, SANDBOX_PORT });

	if (!ws.Run())
		NET_LOG_ERROR(CSTRING("UNABLE TO RUN WEBSERVER"));

	while (ws.IsRunning())
	{
#ifdef BUILD_LINUX
		usleep(1000);
#else
#ifdef NET_X64
		SetConsoleTitle(Net::String(CSTRING("Peer(s): %d | Peer-Threads: %llu"), ws.count_peers_all(), ws.count_pools()).get().get());
#else
		SetConsoleTitle(Net::String(CSTRING("Peer(s): %d | Peer-Threads: %lu"), ws.count_peers_all(), ws.count_pools()).get().get());
#endif
		Sleep(1000);
#endif
	}

	NET_UNLOAD;

	return 0;
}
