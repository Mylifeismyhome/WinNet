#pragma once

#define NET_TOTP Net::Coding::TOTP
#define SHA_BLOCKSIZE 64

#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <assert.h>

NET_DSA_BEGIN
namespace Net
{
	namespace Coding
	{
		namespace TOTP
		{
			uint32_t generateToken(const byte*, size_t, time_t, const int = 30);
		}
	}
}
NET_DSA_END
