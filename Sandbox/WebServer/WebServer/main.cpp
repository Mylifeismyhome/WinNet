#include "config.h"
#include "WebServer/WebServer.h"
#include <Net/assets/manager/logmanager.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore.lib")
#pragma comment(lib, "NetWebServer.lib")
#endif

int main()
{
	Net::load();

	WebServer server;
	server.SetSocketOption<bool>({ TCP_NODELAY, true });
	server.SetOption<char*>({ NET_OPT_NAME, (char*)SANDBOX_SERVERNAME });
	server.SetOption<u_short>({ NET_OPT_PORT, SANDBOX_PORT });

	if (!server.Run())
		LOG_ERROR(CSTRING("UNABLE TO RUN WEBSERVER"));

	while (server.IsRunning())
	{
#ifdef BUILD_LINUX
		usleep(1000);
#else
		SetConsoleTitle(Net::String(CSTRING("Peer(s): %d"), server.getCountPeers()).get().get());
		Sleep(1000);
#endif
	}

	END_LOG;

	Net::unload();

	return 0;
}
