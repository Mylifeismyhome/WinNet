#include "config.h"
#include "Server/Server.h"

#pragma comment(lib, "NetServer.lib")

int main()
{
	Net::load();

	BEGIN_LOG("test2/test/test");

	Server server;
	server.SetOption<char*>({ NET_OPT_NAME, (char*)SANDBOX_SERVERNAME });
	server.SetOption<u_short>({ NET_OPT_PORT, SANDBOX_PORT });
	server.SetOption<bool>({ NET_OPT_USE_CIPHER, true });

	if (!server.Run())
		MessageBoxA(nullptr, CSTRING("Failure on starting Server"), CSTRING("Startup failure"), MB_OK | MB_ICONERROR);
	else
	{
		while (server.IsRunning())
		{
			Sleep(1000);
		}
	}

	END_LOG;

	Net::unload();

	return 0;
}