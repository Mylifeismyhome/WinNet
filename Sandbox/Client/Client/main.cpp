#include <Net/Net/Net.h>
#include "config.h"
#include "Client/Client.h"
#include <Net/assets/web/http.h>
#include <Net/Protocol/NTP.h>
#include <Net/Coding/BASE32.h>
#include <Net/Coding/TOTP.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore.lib")
#pragma comment(lib, "NetClient.lib")
#endif

int main()
{
	Net::load();

	Client client;
	client.SetSocketOption<bool>({ TCP_NODELAY, true });

#ifdef _USE_CIPHER_
	client.SetOption<bool>({ NET_OPT_USE_CIPHER, true });

	LOG_WARNING("USING CIPHER");
#endif

	if (!client.Connect(SANDBOX_SERVERIP, SANBOX_PORT))
	{
		LOG_ERROR(CSTRING("Connection timeout"));
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
