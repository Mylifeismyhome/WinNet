#include "config.h"
#include "Server/Server.h"

#pragma comment(lib, "NetServer.lib")

void OnLogCallback(NET_ON_LOG_PARAMETERS)
{
	std::cout << "OK WE ARE GETTING CALLBACKED!!" << std::endl;
}

int main()
{
	Net::load();

	BEGIN_LOG("test2/test/test");

	NET_ON_LOG(OnLogCallback);

	Server server;
	server.SetSocketOption<bool>({ TCP_NODELAY, true });
	server.SetOption<char*>({ NET_OPT_NAME, (char*)SANDBOX_SERVERNAME });
	server.SetOption<u_short>({ NET_OPT_PORT, SANDBOX_PORT });
	server.SetOption<bool>({ NET_OPT_USE_CIPHER, true });
	server.SetOption<bool>({ NET_OPT_USE_TOTP, true });
//	server.SetOption<int>({ NET_OPT_TOTP_INTERVAL, 30000 });
	server.SetOption<bool>({ NET_OPT_USE_NTP, true });
	server.SetOption<char*>({ NET_OPT_NTP_HOST, (char*)CSTRING("2001:4860:4806:4::") });

	if (!server.Run())
		MessageBoxA(nullptr, CSTRING("Failure on starting Server"), CSTRING("Startup failure"), MB_OK | MB_ICONERROR);
	else
	{
		while (server.IsRunning())
		{
			SetConsoleTitle(NetString("Peer(s): %d", server.getCountPeers()).get().get());
			Sleep(1000);
		}
	}

	END_LOG;

	Net::unload();

	return 0;
}