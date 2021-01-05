#pragma once
#define NET_FA2 Net::Coding::FA2
#define SHA_BLOCKSIZE 64

#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>
#include <assert.h>

NET_DSA_BEGIN
NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_NAMESPACE_BEGIN(TOTP)
uint32_t generateToken(const byte*, size_t, time_t, const int& = 30);
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_DSA_END