#define TEST(name, fnc) void name () { LOG(CSTRING("---------------------------------------")); LOG(CSTRING("Test Case: " #name)); LOG(CSTRING("----------------------------------------")); fnc LOG(CSTRING("----------------------------------------")); }
#define RUN(name) name ();
#define NTP_HOST "129.250.35.251"

#include <Net/Net/Net.h>
#include <Net/Net/NetString.h>

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

TEST(Basic,
);

TEST(Hex,
 	const char* word = "Ich versuche mich mal auf Hex";
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

	LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Hex::encode(data, out, size);

        LOG(CSTRING("Hex Encoded: %s"), out);

        Net::Coding::Hex::decode(out, data, size);

	LOG(CSTRING("Hex Decoded: %s"), data);

	FREE(data);
);

TEST(Base32,
	const char* word = "Ich versuche mich mal auf Base32";
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

        LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Base32::encode(data, out, size);

        LOG(CSTRING("Base32 Encoded: %s"), out);

        Net::Coding::Base32::decode(out, data, size);

        LOG(CSTRING("Base32 Decoded: %s"), data);

        FREE(data);
);

TEST(Base64,
        const char* word = "Ich versuche mich mal auf Base64";
        size_t size = strlen(word);

        BYTE* data = ALLOC<BYTE>(size + 1);
        memcpy(data, word, size);
        data[size] = '\0';

        LOG(CSTRING("Original: %s"), data);

        byte* out = nullptr;
        Net::Coding::Base64::encode(data, out, size);

        LOG(CSTRING("Base64 Encoded: %s"), out);

        Net::Coding::Base64::decode(out, data, size);

        LOG(CSTRING("Base64 Decoded: %s"), data);

        FREE(data);
);

TEST(MD5,
	auto buffer = NET_MD5::createHash((char*)CSTRING("Affe"), (char*)CSTRING("Pferd"), (char*)CSTRING("Maulwurf"));
	LOG(CSTRING("MD5: %s"), buffer);
	FREE(buffer);
);

TEST(SHA1,
	std::string buffer(CSTRING("SHA1 ME!"));

	NET_SHA1 sha1;
	for(const auto& c : buffer)
		sha1 << c;

	unsigned int result;
	sha1.Result(&result);

	LOG(CSTRING("SHA1: %u"), result);
);

TEST(TOTP,
	const auto result = NET_TOTP::generateToken((byte*)"test", 4, time(nullptr), 30);
	LOG(CSTRING("TOTP: %llu"), result);
);

TEST(AES,
	const char* plain = "Ich bin verschlüsselt wurden!";

	size_t size = strlen(plain);
	auto buffer = ALLOC<byte>(size + 1);
	memcpy(buffer, plain, size);
	buffer[size] = '\0';

	LOG(CSTRING("Original: %s"), buffer);

	NET_AES aes;
	aes.init((char*)CSTRING("Affe Hund Bauer"), (char*)CSTRING("Baum"));

	aes.encryptHex(buffer, size);

	LOG(CSTRING("Encrypted: %s"), buffer);

	aes.decryptHex(buffer, size);

	LOG(CSTRING("Decrypted: %s"), buffer);

	FREE(buffer);
);

TEST(rsa,
 	const char* plain = "Ich bin verschlüsselt wurden!";

        size_t size = strlen(plain);
        auto buffer = ALLOC<byte>(size + 1);
        memcpy(buffer, plain, size);
        buffer[size] = '\0';

        LOG(CSTRING("Original: %s"), buffer);

	NET_RSA rsa;
        rsa.generateKeys(2048, 3);

       	rsa.encryptHex(buffer, size);

        LOG(CSTRING("Encrypted: %s"), buffer);

        rsa.decryptHex(buffer, size);

        LOG(CSTRING("Decrypted: %s"), buffer);

	 const auto PublicKey = rsa.publicKey();
	LOG(CSTRING("PKEY: %s"), PublicKey.get());
        FREE(buffer);
);

TEST(Directory,
	const char* dirname = CSTRING("testdir/test");
	if(NET_DIRMANAGER::folderExists(dirname))
	{
		LOG(CSTRING("FOLDER '%s' DOES EXSITS!"), dirname);
	}
	else {
		LOG(CSTRING("FOLDER '%s' DOES NOT EXSITS!"), dirname);
	}

	const auto res = NET_DIRMANAGER::createDir((char*)CSTRING("duhuan\\teste\\mich\\bitte"));
	if(res.error) LOG_ERROR(CSTRING("ERROR ON CREATING DIR!"));
	NET_DIRMANAGER::deleteDir((char*)CSTRING("duhuan/teste/mich/bitte"));

 	NET_FILES sfiles;
        NET_SCANDIR("testdir", sfiles);

        for(const auto& entry : sfiles)
        {
                LOG("%s -- %llu", entry.fullPath, entry.creationTime);

		NET_FILEMANAGERA fmanager(entry.fullPath, NET_FILE_READ);

		byte* data = nullptr;
		size_t size = NULL;
		if(fmanager.read(data, size))
		{
			LOG_WARNING("%s", data);
			FREE(data);
		}
		else
		{
			LOG(CSTRING("CANT READ FILE!"));
		}
        }
);

TEST(HTTP,
	NET_HTTP http(CSTRING("http://localhost/ZAPI/site/server/buyitem.php"));
	if(http.Get())
	{
		LOG("GET WORKED!");
		//LOG(http.GetBodyContent().c_str());
	}
	else
	{
		LOG("GET FAILED!");
	}
);

TEST(HTTPS,
	NET_HTTPS https(CSTRING("https://bs.to/"));
	if(https.Get())
	{
                LOG("GET WORKED!");
               	LOG(https.GetBodyContent().c_str());
        }
        else
        {
                LOG("GET FAILED!");
        }
);

NET_TIMER(TestTimer)
{
	LOG("NET TIMER CALLED!");
	NET_CONTINUE_TIMER;
}

TEST(TIMER,
	const auto handle = Net::Timer::Create(TestTimer, 1000);
	Net::Timer::WaitTimerFinished(handle); // infinite loop in our case
);

TEST(DATABASE,
	MYSQL_CON cfg("localhost", 3306, "root", "", "opldev");
	MYSQL mysql(cfg);
	if (mysql.connect())
	{
		LOG(CSTRING("MYSQL CONNECTED!"));
	}
);

TEST(NTP,
	auto time = Net::Protocol::NTP::Exec(CSTRING(NTP_HOST));

        if (!time.valid())
        {
                LOG_ERROR(CSTRING("critical failure on calling NTP host"));
                return;
        }

        const auto curtime = (time_t)(time.frame().txTm_s - NTP_TIMESTAMP_DELTA);
        const auto sTm = gmtime(&curtime);

        char buff[20];
        strftime(buff, sizeof(buff), CSTRING("%Y-%m-%d %H:%M:%S"), sTm);
        LOG(buff);
);

int main()
{
	RUN(Basic);
	RUN(Hex);
	RUN(Base32);
	RUN(Base64);
	RUN(MD5);
	RUN(SHA1);
	RUN(TOTP);
	RUN(AES);
	RUN(rsa);
	RUN(Directory);
	RUN(HTTP);
	RUN(HTTPS);
	//RUN(TIMER);
	RUN(DATABASE);
	RUN(NTP);

	return 0;
}
