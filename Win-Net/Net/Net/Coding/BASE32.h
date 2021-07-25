#pragma once
#define NET_BASE32_PATTERN (byte*)CSTRING("123456789ABCDEFGHJKMNPQRSTUVWXYZ")
#define NET_BASE32 Net::Coding::Base32

#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>

NET_DSA_BEGIN
NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_NAMESPACE_BEGIN(Base32)
bool encode(byte*&, size_t&);
bool encode(byte*, byte*&, size_t&);
bool decode(byte*&, size_t&);
bool decode(byte*, byte*&, size_t&);
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_DSA_END
