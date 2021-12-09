#include "config.h"
#include "Server/Server.h"
#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <Net/assets/manager/logmanager.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore.lib")
#pragma comment(lib, "NetServer.lib")
#endif

int main()
{
	Net::load();

	Server server;
	server.SetOption<char*>({ NET_OPT_NAME, (char*)SANDBOX_SERVERNAME });
	server.SetOption<u_short>({ NET_OPT_PORT, SANDBOX_PORT });

#ifdef _USE_CIPHER_
	server.SetOption<bool>({ NET_OPT_USE_CIPHER, true });
	LOG_WARNING(CSTRING("USING CIPHER"));
#endif

	if (!server.Run())
		LOG_ERROR(CSTRING("UNABLE TO RUN SERVER"));

	while (server.IsRunning())
	{
#ifdef BUILD_LINUX
		usleep(1000);
#else
		SetConsoleTitle(Net::String(CSTRING("Peer(s): %d | Peer-Threads: %llu"), server.count_peers_all(), server.count_pools()).get().get());
		Sleep(1000);
#endif
	}

	Net::unload();

	return 0;
}
