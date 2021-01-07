#include <Net/Net/Net.h>
#include "config.h"
#include "Client/Client.h"
#include <Net/assets/web/http.h>
#include <Net/Protocol/NTP.h>
#include <Net/Coding/BASE32.h>
#include <Net/Coding/2FA.h>

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

	/*auto time = Net::Protocol::NTP::Exec("2001:4860:4806:4::");
	if (!time.valid())
		return 0;

	auto curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);

	tm tm;
	gmtime_s(&tm, &curTime);
	tm.tm_hour = Net::Util::roundUp(tm.tm_hour, 10);
	tm.tm_min = Net::Util::roundUp(tm.tm_min, 10);
	tm.tm_sec = 0;
	const auto txTm = mktime(&tm);

	const auto strTime = ctime(&txTm);
	auto fa2_secret_len = strlen(strTime);

	auto fa2_secret = ALLOC<byte>(fa2_secret_len + 1);
	memcpy(fa2_secret, strTime, fa2_secret_len);
	fa2_secret[fa2_secret_len] = '\0';
	Net::Coding::Base32::base32_encode(fa2_secret, fa2_secret_len);

	while (true)
	{
		time = Net::Protocol::NTP::Exec("2001:4860:4806:4::");
		if (!time.valid())
			return 0;

		curTime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);

		LOG("%d", Net::Coding::FA2::generateToken(fa2_secret, fa2_secret_len, curTime));

		Sleep(1000);
	}*/

	/*const char* str = "ddn45nd5dr";
	auto len = strlen(str);
	byte* out = new byte[len + 1];
	memcpy(out, str, len);
	out[len] = '\0';
	Net::Coding::Base32::base32_encode(out, len);

	do
	{
		std::cout << Net::Coding::TOTP::generateToken(out, len, time(nullptr), 30) << std::endl;
		Sleep(1000);
	} while (true);*/

	Client client;
	client.SetSocketOption<bool>({ TCP_NODELAY, true });
	client.SetOption<bool>({ NET_OPT_USE_CIPHER, true });
	client.SetOption<bool>({ NET_OPT_USE_TOTP, false });
	//client.SetOption<int>({ NET_OPT_TOTP_INTERVAL, 30000 });
	client.SetOption<bool>({ NET_OPT_USE_NTP, true });
	client.SetOption<char*>({ NET_OPT_NTP_HOST, (char*)CSTRING("2001:4860:4806:4::") });

	if (!client.Connect(SANDBOX_SERVERIP, SANBOX_PORT))
		MessageBoxA(nullptr, CSTRING("Connection timeout"), CSTRING("Network failure"), MB_OK | MB_ICONERROR);
	else
	{
		while (client.IsConnected())
		{
			Sleep(1000);
		}
	}

	Net::unload();

	return 0;
}