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

	Net::Json::Document doc;
	doc["Test"] = 5;
	doc["ABC"]["DCE"] = 25.12f;

	Net::Json::Array arr;
	arr.push(25);
	arr.push("TEST");
	arr.push(13.125f);

	doc["ARR"] = arr;

	std::cout << doc["ARR"]->as_array()->operator[](1)->as_string() << std::endl;
	std::cout << doc["ARR"]->as_array()->at(1)->as_string() << std::endl;

	std::cout << doc.Serialize(Net::Json::SerializeType::FORMATTED).get().get() << std::endl;

	// new doc
	doc = Net::Json::Document();
	doc.Deserialize(R"({
        "Test" : 5,
        "ABC" : {
                "DCB" : 25.120001

        },
        "ARR" : [
                25,
                "TEST2",
                13.125000

        ]

})");

	std::cout << doc.Serialize(Net::Json::SerializeType::FORMATTED).get().get() << std::endl;

	Client client;

#ifdef _USE_CIPHER_
	client.SetOption<bool>({ NET_OPT_USE_CIPHER, true });

	LOG_WARNING(CSTRING("USING CIPHER"));
#endif

	client.SetOption<bool>({ NET_OPT_USE_COMPRESSION, true });

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
