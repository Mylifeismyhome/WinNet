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
			bool init(char*, char* = nullptr, size_t = CryptoPP::AES::DEFAULT_KEYLENGTH);

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