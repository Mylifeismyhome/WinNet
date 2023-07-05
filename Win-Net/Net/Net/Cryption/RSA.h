/*
	MIT License

	Copyright (c) 2022 Tobias Staack

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

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

			void setPublicKey(char*);
			void setPublicKey(const char*);
			void setPrivateKey(char*);
			void setPrivateKey(const char*);

			void deleteKeys();

			bool init(const char*, const char*);
			bool init(char*, char* = nullptr);
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