#pragma once

#define NET_AES Net::Cryption::AES

#include <Net/Net/Net.h>

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>

#include <Net/Cryption/XOR.h>
#include <Net/Coding/Hex.h>
#include <Net/Coding/BASE64.h>
#include <Net/assets/assets.h>

NET_DSA_BEGIN
namespace Net
{
	namespace Cryption
	{
		class AES
		{
			/* STORE KEYS */
			RUNTIMEXOR Key;
			RUNTIMEXOR IV;
			size_t KeyLength;

			/* ACTUALL ENC/DEC FUNC */
			bool encrypt(CryptoPP::byte*, const size_t, const char*, const char*) const;
			bool decrypt(CryptoPP::byte*, const size_t, const char*, const char*) const;

		public:
			AES();
			~AES();

			/* FIRST INIT AES */
			bool init(const char*, const char*, size_t = CryptoPP::AES::DEFAULT_KEYLENGTH);
			bool init(char*, bool = false, char* = nullptr, bool = false, size_t = CryptoPP::AES::DEFAULT_KEYLENGTH);

			bool encrypt(CryptoPP::byte*, size_t);
			bool encryptHex(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
			bool encryptHex(CryptoPP::byte*&, size_t&);
			bool encryptBase64(CryptoPP::byte*&, size_t&);
			bool encryptBase64(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
			bool decrypt(CryptoPP::byte*, size_t);
			bool decryptHex(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
			bool decryptHex(CryptoPP::byte*&, size_t&);
			bool decryptBase64(CryptoPP::byte*&, size_t&);
			bool decryptBase64(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
		};
	}
}
NET_DSA_END