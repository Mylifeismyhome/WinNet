#pragma once

#define NET_HEX Net::Coding::Hex

#include <Net/Net/Net.h>
#include <cryptopp/hex.h>
#include <Net/Cryption/XOR.h>
#include <Net/assets/assets.h>

namespace Net
{
	namespace Coding
	{
		namespace Hex
		{
			bool encode(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
			bool encode(CryptoPP::byte*&, size_t&);
			bool decode(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
			bool decode(CryptoPP::byte*&, size_t&);
		}
	}
}