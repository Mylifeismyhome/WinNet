#include "config.h"
#include "Server/Server.h"
#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <Net/assets/manager/logmanager.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore_static.lib")
#pragma comment(lib, "NetServer_static.lib")
#endif

int main()
{
	Net::load(Net::ENABLE_LOGGING);

	Server server;
	server.SetOption<char*>({ NET_OPT_NAME, (char*)SANDBOX_SERVERNAME });
	server.SetOption<u_short>({ NET_OPT_PORT, SANDBOX_PORT });
	server.SetOption<bool>({ NET_OPT_EXECUTE_PACKET_ASYNC, false });
	server.SetOption<bool>({ NET_OPT_USE_COMPRESSION, true });
	server.SetOption<bool>({ NET_OPT_USE_TOTP, true });
	server.SetOption<bool>({ NET_OPT_USE_NTP, false });

#ifdef _USE_CIPHER_
	server.SetOption<bool>({ NET_OPT_USE_CIPHER, true });
	NET_LOG_WARNING(CSTRING("USING CIPHER"));
#endif

	if (!server.Run())
		NET_LOG_ERROR(CSTRING("UNABLE TO RUN SERVER"));

	while (server.IsRunning())
	{
#ifdef BUILD_LINUX
		usleep(1000);
#else
#ifdef NET_X64
		SetConsoleTitle(Net::String(CSTRING("Peer(s): %d | Peer-Threads: %llu"), server.count_peers_all(), server.count_pools()).get().get());
#else
		SetConsoleTitle(Net::String(CSTRING("Peer(s): %d | Peer-Threads: %lu"), server.count_peers_all(), server.count_pools()).get().get());
#endif
		Sleep(1000);
#endif
	}

	Net::unload();

	return 0;
}
