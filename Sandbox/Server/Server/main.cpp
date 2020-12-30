#include "config.h"
#include "Server/Server.h"

#pragma comment(lib, "NetServer.lib")

int main()
{
	Net::load();

	//BEGIN_LOG("test2/test/test");

	Server server;
	server.SetOption<char*>({ OPT_ServerName, (char*)SANDBOX_SERVERNAME });
	server.SetOption<u_short>({ OPT_ServerPort, SANDBOX_PORT });
	server.SetOption<bool>({ OPT_CryptPackage, true });
	if (!server.Run())
		MessageBoxA(nullptr, CSTRING("Failure on starting Server"), CSTRING("Startup failure"), MB_OK | MB_ICONERROR);
	else
	{
		while (server.IsRunning())
		{
			Sleep(1000);
		}
	}

	//END_LOG;

	Net::unload();

	return 0;
}