#include "config.h"
#include "Server/Server.h"
#include <Net/assets/manager/logmanager.h>

#include <Net/Compression/Compression.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore.lib")
#pragma comment(lib, "NetServer.lib")
#endif

int main()
{
	Net::load();

	NET_FILEMANAGER fmanager("test.exe", NET_FILE_READ);

	byte* data = nullptr;
	size_t size = 0;
	if (!fmanager.read(data, size)) return 10;

	NET_LZMA::Compress(data, size);


	{

		NET_FILEMANAGER fmanager2("test.out", NET_FILE_READWRITE);
		fmanager2.write(data, size);
	}

	FREE(data);

	Server server;
	server.SetOption<char*>({ NET_OPT_NAME, (char*)SANDBOX_SERVERNAME });
	server.SetOption<u_short>({ NET_OPT_PORT, SANDBOX_PORT });
	server.SetOption<bool>({ NET_OPT_USE_COMPRESSION, true });

#ifdef _USE_CIPHER_
	server.SetOption<bool>({ NET_OPT_USE_CIPHER, true });
	LOG_WARNING(CSTRING("USING CIPHER"));
#endif

	if (!server.Run())
		LOG_ERROR(CSTRING("UNABLE TO RUN SERVER"));

	while (server.IsRunning())
	{
#ifdef BUILD_LINUX
		usleep(1000);
#else
		SetConsoleTitle(Net::String(CSTRING("Peer(s): %d"), server.getCountPeers()).get().get());
		Sleep(1000);
#endif
	}

	END_LOG;

	Net::unload();

	return 0;
}
