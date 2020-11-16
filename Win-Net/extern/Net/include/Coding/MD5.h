#pragma once
#include <Net/Net/Net.h>
#include <Coding/md5/md5_native.h>

NET_DSA_BEGIN

#define NET_MD5 Net::Coding::MD5

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_NAMESPACE_BEGIN(MD5)
char* createHash(const char*, const char*, char*);
long long longParse(char*);
char* createMD5Buffer(int, int, char[8]);
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END