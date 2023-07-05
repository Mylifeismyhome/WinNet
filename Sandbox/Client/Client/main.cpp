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

	/*__debugbreak();
	{
		Net::Json::Document doc;
		doc.Parse(R"({"test":3})");

		{
			Net::Json::Document doc2;
			doc2["abbb"] = 5;
			doc2["a"] = doc;

			auto str = doc2.Serialize();
			printf(str.get().get());
		}
	}
	__debugbreak();*/

	Client client;
	client.SetOption<bool>({ NET_OPT_EXECUTE_PACKET_ASYNC, false });

	if (!client.Connect(SANDBOX_SERVERIP, SANBOX_PORT))
	{
		NET_LOG_ERROR(CSTRING("Connection timeout"));
	}
	else
	{
		while (client.getConnectionStatus() != Net::Client::EDISCONNECTED)
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
