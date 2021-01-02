#pragma once
#define NET_BASE32_PATTERN (byte*)CSTRING("123456789ABCDEFGHJKMNPQRSTUVWXYZ")
#define NET_BASE32 Net::Coding::Base32

#include <Net/Net/Net.h>
#include <Net/Cryption/XOR.h>

NET_DSA_BEGIN
NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_NAMESPACE_BEGIN(Base32)
bool Decode32(unsigned char* in, int inLen, unsigned char* out);
bool Encode32(unsigned char* in, int inLen, unsigned char* out);
int GetDecode32Length(int bytes);
int GetEncode32Length(int bytes);
bool Map32(unsigned char* inout32, int inout32Len, unsigned char* alpha32);
bool Unmap32(unsigned char* inout32, int inout32Len, unsigned char* alpha32);

bool base32_encode(byte*&, size_t&);
bool base32_decode(byte*&, size_t&);
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_DSA_END