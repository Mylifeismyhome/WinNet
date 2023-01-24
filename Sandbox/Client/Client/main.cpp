#include <Net/Net/Net.h>
#include "config.h"
#include "Client/Client.h"
#include <Net/assets/web/http.h>
#include <Net/Protocol/NTP.h>
#include <Net/Coding/BASE32.h>
#include <Net/Coding/TOTP.h>
#include <Net/Net/NetJson.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore_static.lib")
#pragma comment(lib, "NetClient_static.lib")
#endif

int main()
{
	Net::load(Net::ENABLE_LOGGING);

	Client client;

#ifdef _USE_CIPHER_
	client.SetOption<bool>({ NET_OPT_USE_CIPHER, true });

	NET_LOG_WARNING(CSTRING("USING CIPHER"));
#endif

	client.SetOption<bool>({ NET_OPT_EXECUTE_PACKET_ASYNC, true });
	client.SetOption<bool>({ NET_OPT_USE_COMPRESSION, true });

	if (!client.Connect(SANDBOX_SERVERIP, SANBOX_PORT))
	{
		NET_LOG_ERROR(CSTRING("Connection timeout"));
	}
	else
	{
		while (client.IsConnected())
		{
#ifdef BUILD_LINUX
			usleep(1000);
#else
			Sleep(1000);
#endif
		}
	}

	Net::unload();

	return 0;
}
