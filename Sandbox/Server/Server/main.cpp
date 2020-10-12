#include "config.h"
#include "Server/Server.h"

#pragma comment(lib, "NetServer.lib")

#include <crypto++/includes/channels.h> 
#include <crypto++/includes/filters.h> 
#include <crypto++/includes/sha.h> 
#include <crypto++/includes/hex.h>
#include <crypto++/includes/files.h>
#include <crypto++/includes/crc.h>

byte* CalcCRC32(byte* input, const size_t len)
{
	CryptoPP::CRC32 crc32;
	size_t size = CryptoPP::CRC32::DIGESTSIZE;
	auto digest = new byte[CryptoPP::CRC32::DIGESTSIZE];
	crc32.CalculateDigest(digest, input, len);
	const NET_HEX hex;
	hex.encodeString(&digest, size);
	return digest;
}

byte* CalcCRC32(char* input, const size_t len)
{
	CryptoPP::CRC32 crc32;
	size_t size = CryptoPP::CRC32::DIGESTSIZE;
	auto digest = new byte[CryptoPP::CRC32::DIGESTSIZE];
	crc32.CalculateDigest(digest, reinterpret_cast<byte*>(input), len);
	const NET_HEX hex;
	hex.encodeString(&digest, size);
	return digest;
}

byte* CalcCRC32(const std::string& input)
{
	CryptoPP::CRC32 crc32;
	size_t size = CryptoPP::CRC32::DIGESTSIZE;
	auto digest = new byte[CryptoPP::CRC32::DIGESTSIZE];
	crc32.CalculateDigest(digest, reinterpret_cast<const byte*>(input.data()), input.length());
	const NET_HEX hex;
	hex.encodeString(&digest, size);
	return digest;
}

int main()
{
	const std::string test("123DU HURENSOHN");
	auto crc = CalcCRC32(test);
	for(size_t i = 0; i < 200; ++i)
	{
		NetString str("%s", crc);
		LOG(str.str().data());
		LOG(str.str().data());
	}
	FREE(crc);
	Server server;
	//server.SetCryptPackage(true);
	if (!server.Start(SANDBOX_SERVERNAME, SANDBOX_PORT))
		MessageBoxA(nullptr, CSTRING("Failure on starting Server"), CSTRING("Startup failure"), MB_OK | MB_ICONERROR);
	else
	{
		while (server.IsRunning())
		{
			Sleep(1000);
		}
	}

	return 0;
}