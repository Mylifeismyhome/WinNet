#pragma once

#define NET_MD5 Net::Coding::MD5

#include <Net/Net/Net.h>
#include <Net/Coding/md5/md5_native.h>

NET_DSA_BEGIN
namespace Net
{
	namespace Coding
	{
		namespace MD5
		{
			char* createHash(const char*, const char*, char*);
			long long longParse(char*);
			char* createMD5Buffer(int, int, char[8]);
		}
	}
}
NET_DSA_END