#include "config.h"
#include "Client/Client.h"
#include <Net/assets/web/http.h>
#include <Net/Protocol/NTP.h>

#pragma comment(lib, "NetClient.lib")

int main()
{
	Net::load();

	// test
	/*const auto host = Net::Protocol::NTP::ResolveHostname("time.google.com");

	// test NTP
	const auto res = Net::Protocol::NTP::Exec(host, 123);
	if (res.valid())
	{
		time_t txTm = (time_t)(res.frame().txTm_s - NTP_TIMESTAMP_DELTA);
		printf("Time: %s", ctime((const time_t*)&txTm));
	}

	system("pause");

	// test http parsing
	Net::Web::HTTPS https("https://google.com", Net::ssl::NET_SSL_METHOD_TLSv1_2_CLIENT);
	if(https.Get())
	{
		
		LOG_ERROR("%s", https.GetHeaderContent().data());
		LOG("%s", https.GetBodyContent().data());
	}
	else
		LOG_ERROR("%s", https.GetRawData().data());
	
	system("pause");*/

	Client client;
	client.SetSocketOption<bool>({ TCP_NODELAY, true });
	client.SetOption<int>({ TCP_NODELAY,  1050});

	if (client.Isset(0x1))
	{
		LOG("%i", client.GetOption<int>(0x1));
	}

	client.SetCryptPackage(true);
	if (!client.Connect(SANDBOX_SERVERIP, SANBOX_PORT))
		MessageBoxA(nullptr, CSTRING("Connection timeout"), CSTRING("Network failure"), MB_OK | MB_ICONERROR);
	else
	{
		while(client.IsConnected())
		{
			Sleep(1000);
		}

		client.WaitUntilAccomplished();
	}

	Net::unload();

	return 0;
}