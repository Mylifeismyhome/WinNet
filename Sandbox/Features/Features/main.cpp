#define TEST(name, fnc) void name () { NET_LOG(CSTRING("---------------------------------------")); NET_LOG(CSTRING("Test Case: " #name)); NET_LOG(CSTRING("----------------------------------------")); fnc NET_LOG(CSTRING("----------------------------------------")); }
#define RUN(name) name ();
#define NTP_HOST "129.250.35.251"

#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>
#include <Net/Net/NetJson.h>

// coding
#include <Net/Coding/Hex.h>
#include <Net/Coding/BASE32.h>
#include <Net/Coding/BASE64.h>
#include <Net/Coding/MD5.h>
#include <Net/Coding/SHA1.h>
#include <Net/Coding/TOTP.h>

// cryption
#include <Net/Cryption/AES.h>
#include <Net/Cryption/RSA.h>

// assets
#include <Net/assets/manager/dirmanager.h>
#include <Net/assets/manager/filemanager.h>
#include <Net/assets/web/http.h>
#include <Net/assets/timer.h>

// database
#include <Net/Database/MYSQL.h>

// Protocol
#include <Net/Protocol/NTP.h>

#ifndef BUILD_LINUX
#pragma comment(lib, "NetCore_static.lib")
#pragma comment(lib, "NetClient_static.lib")
#endif

TEST(Basic,
);

TEST(Hex,
 		const char* word = CSTRING("Ich versuche mich mal auf Hex");
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

		NET_LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Hex::encode(data, out, size);

		NET_LOG(CSTRING("Hex Encoded: %s"), out);

        Net::Coding::Hex::decode(out, data, size);

		NET_LOG(CSTRING("Hex Decoded: %s"), data);

		FREE(data);
);

TEST(Base32,
		const char* word = CSTRING("Ich versuche mich mal auf Base32");
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

		NET_LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Base32::encode(data, out, size);

		NET_LOG(CSTRING("Base32 Encoded: %s"), out);

        Net::Coding::Base32::decode(out, data, size);

		NET_LOG(CSTRING("Base32 Decoded: %s"), data);

        FREE(data);
);

TEST(Base64,
        const char* word = CSTRING("Ich versuche mich mal auf Base64");
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

		NET_LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Base64::encode(data, out, size);

		NET_LOG(CSTRING("Base64 Encoded: %s"), out);

        Net::Coding::Base64::decode(out, data, size);

		NET_LOG(CSTRING("Base64 Decoded: %s"), data);

        FREE(data);
);

TEST(MD5,
		BYTE* buffer = (BYTE*)NET_MD5::createHash((char*)CSTRING("Affe"), (char*)CSTRING("Pferd"), (char*)CSTRING("Maulwurf"));
		size_t len = strlen((const char*)buffer);

		Net::Coding::Hex::encode(buffer, len);

		NET_LOG(CSTRING("MD5: %s"), buffer);

		FREE(buffer);
);

TEST(SHA1,
		std::string buffer(CSTRING("SHA1 ME!"));

		NET_SHA1 sha1;
		for(const auto& c : buffer)
			sha1.add(c);

		sha1.finalize();

		auto result = ALLOC<char>(NET_SHA1_HEX_SIZE);
		sha1.to_hex(result);

		NET_LOG(CSTRING("SHA1: %s"), result);

		FREE(result);
);

TEST(TOTP,
		const auto result = NET_TOTP::generateToken((byte*)CSTRING("test"), 4, time(nullptr), 30);
		NET_LOG(CSTRING("TOTP: %llu"), result);
);

TEST(AES,
	const char* plain = CSTRING("Ich bin verschluesselt wurden!");

	size_t size = strlen(plain);
	auto buffer = ALLOC<byte>(size + 1);
	memcpy(buffer, plain, size);
	buffer[size] = '\0';

	NET_LOG(CSTRING("Original: %s"), buffer);

	NET_AES aes;
	aes.init((char*)CSTRING("Affe Hund Bauer"), (char*)CSTRING("Baum"));

	aes.encryptHex(buffer, size);

	NET_LOG(CSTRING("Encrypted: %s"), buffer);

	aes.decryptHex(buffer, size);

	NET_LOG(CSTRING("Decrypted: %s"), buffer);

	FREE(buffer);
);

TEST(rsa,
 		const char* plain = CSTRING("Ich bin verschluesselt wurden!");

        size_t size = strlen(plain);
        auto buffer = ALLOC<byte>(size + 1);
        memcpy(buffer, plain, size);
        buffer[size] = '\0';

		NET_LOG(CSTRING("Original: %s"), buffer);

		NET_RSA rsa;
        rsa.generateKeys(2048, 3);

       	rsa.encryptHex(buffer, size);

		NET_LOG(CSTRING("Encrypted: %s"), buffer);

        rsa.decryptHex(buffer, size);

		NET_LOG(CSTRING("Decrypted: %s"), buffer);

		const auto PublicKey = rsa.publicKey();
		NET_LOG(CSTRING("PKEY: %s"), PublicKey.get());
		FREE(buffer);
);

TEST(Directory,
		const char* dirname = CSTRING("testdir/test");
		if(NET_DIRMANAGER::folderExists(dirname))
		{
			NET_LOG(CSTRING("FOLDER '%s' DOES EXSITS!"), dirname);
		}
		else {
			NET_LOG(CSTRING("FOLDER '%s' DOES NOT EXSITS!"), dirname);
		}

		const auto res = NET_DIRMANAGER::createDir((char*)CSTRING("test\\teste\\mich\\bitte"));
		if(res.error) NET_LOG_ERROR(CSTRING("ERROR ON CREATING DIR!"));
		NET_DIRMANAGER::deleteDir((char*)CSTRING("test/teste/mich/bitte"));

 		NET_FILES sfiles;
        NET_SCANDIR(CSTRING("testdir"), sfiles);

        for(const auto& entry : sfiles)
        {
			NET_LOG(CSTRING("%s -- %llu"), entry.fullPath, entry.creationTime);

			NET_FILEMANAGERA fmanager(entry.fullPath, NET_FILE_READ);

			byte* data = nullptr;
			size_t size = NULL;
			if(fmanager.read(data, size))
			{
				NET_LOG_WARNING(CSTRING("%s"), data);
				FREE(data);
			}
			else
			{
				NET_LOG(CSTRING("CANT READ FILE!"));
			}
		}
);

TEST(HTTP,
		NET_HTTP http(CSTRING("http://localhost/"));
		if(http.Get())
		{
			NET_LOG(CSTRING("GET WORKED!"));
			NET_LOG(http.GetBodyContent().c_str());
		}
		else
		{
			NET_LOG(CSTRING("GET FAILED!"));
			NET_LOG(http.GetBodyContent().c_str());
		}
);

TEST(HTTPS,
		NET_HTTPS https(CSTRING("https://bs.to/"));
		if(https.Get())
		{
			NET_LOG(CSTRING("GET WORKED!"));
			NET_LOG(https.GetHeaderContent().c_str());
			NET_LOG(https.GetBodyContent().c_str());
		}
		else
		{
			NET_LOG(CSTRING("GET FAILED!"));
			NET_LOG(https.GetHeaderContent().c_str());
			NET_LOG(https.GetBodyContent().c_str());
		}
);

NET_TIMER(TestTimer)
{
		NET_LOG(CSTRING("NET TIMER CALLED!"));
		NET_STOP_TIMER;
}

TEST(TIMER,
		const auto handle = Net::Timer::Create(TestTimer, 1000);
		Net::Timer::WaitTimerFinished(handle); // infinite loop in our case
);

TEST(DATABASE,
		//MYSQL_CON cfg(CSTRING("localhost"), 3306, CSTRING("root"), CSTRING(""), CSTRING("test"));
		//MYSQL mysql(cfg);
		//if (mysql.connect()) // mysql connector throwing error
		//{
		//	NET_LOG(CSTRING("MYSQL CONNECTED!"));
		//}
);

TEST(NTP,
		auto time = Net::Protocol::NTP::Exec(CSTRING(NTP_HOST));

        if (!time.valid())
        {
			NET_LOG_ERROR(CSTRING("critical failure on calling NTP host"));
			return;
        }

        const auto curtime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);
        const auto sTm = gmtime(&curtime);

        char buff[20];
        strftime(buff, sizeof(buff), CSTRING("%Y-%m-%d %H:%M:%S"), sTm);
		NET_LOG(buff);
);

Net::String gets()
{
	Net::String str;
	str += "test222";
	return str;
}

int main()
{
	NET_INITIALIZE(Net::ENABLE_LOGGING);

	{
		Net::String str(R"(["test","hacker",["aka46","test nigger"],"aka",true,null,1337,1.235])");
		auto vs = str.view_string(0);
		
		Net::Json::Array arr;
		arr.Deserialize(vs, false);

		std::cout << arr.Stringify(Net::Json::SerializeType::UNFORMATTED).get().get() << std::endl;
	}

	NET_FILEMANAGER f(Net::String("%s/test.json", Net::Manager::Directory::homeDirA().data()).get().get(), NET_FILE_READ);

	char* data = nullptr;
	if (!f.read(data))
	{
		NET_LOG("failed reading file");
		system("pause");
		return 0;
	}

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	Net::Json::Document doc;
	if (!doc.Parse(data))
	{
		NET_LOG("Failed json deserialize");
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;

	system("pause");

	begin = std::chrono::steady_clock::now();

	auto outstr = doc.Stringify(Net::Json::SerializeType::UNFORMATTED);
	NET_FILEMANAGER fb("test.out", NET_FILE_READ | NET_FILE_WRITE);
	fb.write(outstr.get().get());

	end = std::chrono::steady_clock::now();

	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "[ms]" << std::endl;

	//FREE(data);
	system("pause");
	//RUN(Basic);
	//RUN(Hex);
	//RUN(Base32);
	//RUN(Base64);
	//RUN(MD5);
	//RUN(SHA1);
	//RUN(TOTP);
	//RUN(AES);
	//RUN(rsa);
	//RUN(Directory);
	//RUN(HTTP);
	//RUN(HTTPS);
	//RUN(TIMER);
	////RUN(DATABASE);
	//RUN(NTP);

	NET_UNLOAD;

	return 0;
}
