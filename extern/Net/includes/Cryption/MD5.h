#pragma once
#include <Net/Net.h>
#include <Cryption/md5/md5.h>

#define NET_MD5 Net::MD5

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Cryption)
BEGIN_NAMESPACE(MD5)
char* createHash(const char*, const char*, char*);
long long longParse(char*);
char* createMD5Buffer(int, int, char[8]);
END_NAMESPACE
END_NAMESPACE
END_NAMESPACE