#include "config.h"
#include "WebSocket/WebSocket.h"
#include <Net/assets/manager/logmanager.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore_static.lib")
#pragma comment(lib, "NetWebSocket_static.lib")
#endif

int main()
{
	Net::load(Net::ENABLE_LOGGING);

	WebSocket ws;
	ws.SetOption<char*>({ NET_OPT_NAME, (char*)SANDBOX_SERVERNAME });
	ws.SetOption<u_short>({ NET_OPT_PORT, SANDBOX_PORT });

	if (!ws.Run())
		LOG_ERROR(CSTRING("UNABLE TO RUN WEBSERVER"));

	while (ws.IsRunning())
	{
#ifdef BUILD_LINUX
		usleep(1000);
#else
		SetConsoleTitle(Net::String(CSTRING("Peer(s): %d"), ws.getCountPeers()).get().get());
		Sleep(1000);
#endif
	}

	END_LOG;

	Net::unload();

	return 0;
}
