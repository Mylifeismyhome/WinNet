#include "config.h"
#include "Server/Server.h"

#pragma comment(lib, "NetServer.lib")

int main()
{
	BEGIN_LOG("TEST")
	
	Server server;
	server.SetCryptPackage(true);
	if (!server.Start(SANDBOX_SERVERNAME, SANDBOX_PORT))
		MessageBoxA(nullptr, CSTRING("Failure on starting Server"), CSTRING("Startup failure"), MB_OK | MB_ICONERROR);
	else
	{
		while (server.IsRunning())
		{
			Sleep(1000);
		}
	}

	END_LOG

	return 0;
}