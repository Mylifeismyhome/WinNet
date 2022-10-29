#pragma once

#define NET_RSA Net::Cryption::NETRSA

#include <Net/Net/Net.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <cryptopp/rsa.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>

#include <Net/Cryption/XOR.h>
#include <Net/assets/assets.h>

#include <Net/Coding/BASE64.h>
#include <Net/Coding/Hex.h>

#include <Net/assets/manager/logmanager.h>

NET_DSA_BEGIN
namespace Net
{
	namespace Cryption
	{
		class NETRSA
		{
			RSA* KeyPair;

			void generateKeyPair(size_t, int);

			// KEYS
			RUNTIMEXOR _PublicKey;
			RUNTIMEXOR _PrivateKey;
			bool Set;

		public:
			NETRSA();
			~NETRSA();

			bool generateKeys(size_t, int);

			XOR_UNIQUEPOINTER publicKey();
			XOR_UNIQUEPOINTER privateKey();

			void setPublicKey(char*, bool = false);
			void setPublicKey(const char*);
			void setPrivateKey(char*, bool = false);
			void setPrivateKey(const char*);

			void deleteKeys();

			bool init(const char*, const char*);
			bool init(char*, bool = false, char* = nullptr, bool = false);
			bool isSet() const { return Set; }

			bool encrypt(CryptoPP::byte*&, size_t&);
			bool encryptHex(CryptoPP::byte*&, size_t&);
			bool encryptBase64(CryptoPP::byte*&, size_t&);
			bool decrypt(CryptoPP::byte*&, size_t&);
			bool decryptHex(CryptoPP::byte*&, size_t&);
			bool decryptBase64(CryptoPP::byte*&, size_t&);
		};
	}
}
NET_DSA_END