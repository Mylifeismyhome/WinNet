#include "config.h"
#include "Server/Server.h"

#pragma comment(lib, "NetServer.lib")

int main()
{
	NET_RSA rsa;
	rsa.GenerateKeys(1024, 3);

	byte* test = new byte[10];
	memset(test, 'a', 10);
	size_t size = 10;
	rsa.encrypt(test, size);
	rsa.decrypt(test, size);
	LOG("%s", test);
	system("pause");
	
	//for(size_t i=  0; i < 10000; ++i)
	//{
	//	std::vector<NET_FILE_ATTR> file;
	//	NET_SCANDIR((char*)NET_CURRENTDIR.data(), file);
	//}
	//
	//BEGIN_LOG("TEST")
	//
	//Server server;
	//server.SetCryptPackage(true);
	//if (!server.Start(SANDBOX_SERVERNAME, SANDBOX_PORT))
	//	MessageBoxA(nullptr, CSTRING("Failure on starting Server"), CSTRING("Startup failure"), MB_OK | MB_ICONERROR);
	//else
	//{
	//	while (server.IsRunning())
	//	{
	//		Sleep(1000);
	//	}
	//}

	//END_LOG

	return 0;
}