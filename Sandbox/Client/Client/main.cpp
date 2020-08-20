#include "config.h"
#include "Client/Client.h"

#pragma comment(lib, "NetClient.lib")

int main()
{
	Client client;
//	client.SetCryptPackage(true);
	if (!client.Connect(SANDBOX_SERVERIP, SANBOX_PORT))
		MessageBoxA(nullptr, CSTRING("Connection timeout"), CSTRING("Network failure"), MB_OK | MB_ICONERROR);
	else
	{
		while(client.IsConnected())
		{
			Sleep(1000);
		}
	}

	return 0;
}