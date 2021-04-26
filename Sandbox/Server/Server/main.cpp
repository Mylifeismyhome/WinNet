#include "config.h"
#include "Server/Server.h"
#include "Net/assets/web/http.h"

#pragma comment(lib, "NetCore.lib")
#pragma comment(lib, "NetServer.lib")

void OnLogCallback(NET_ON_LOG_PARAMETERS)
{
	std::cout << "OK WE ARE GETTING CALLBACKED!!" << std::endl;
}

int main()
{
	Net::load();

	Net::Web::HTTP http("http://144.91.101.219/ZAPI/site/client/profile.php?session=dd8870c3e6ed5dc9ee3157bc8c432e2e8d62c576cfb27b9e1a9cc59244811aa9");
	if (!http.Get())
	{
		LOG("FUCK");
		return getchar();
	}

	LOG(http.GetBodyContent().data());
	return getchar();


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