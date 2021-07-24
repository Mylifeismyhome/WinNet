#pragma once
#define NET_BASE64_PATTERN CSTRING("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
#define NET_BASE64 Net::Coding::Base64

#include <Net/Net/Net.h>
#include <crypto++/includes/base64.h>
#include <Net/Cryption/XOR.h>
#include <cstdio>
#include <Net/assets/manager/logmanager.h>

NET_DSA_BEGIN
NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_NAMESPACE_BEGIN(Base64)
bool encode(BYTE*, BYTE*&, size_t&);
bool encode(BYTE*&, size_t&);
bool decode(BYTE*, BYTE*&, size_t&);
bool decode(BYTE*&, size_t&) ;
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_DSA_END