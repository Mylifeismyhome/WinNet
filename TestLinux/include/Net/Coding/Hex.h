#pragma once
#define NET_HEX Net::Coding::Hex

#include <Net/Net/Net.h>
#include <crypto++/includes/hex.h>
#include <Net/Cryption/XOR.h>
#include <Net/assets/assets.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_NAMESPACE_BEGIN(Hex)
bool encode(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
bool encode(CryptoPP::byte*&, size_t&);
bool decode(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
bool decode(CryptoPP::byte*&, size_t&);
NET_NAMESPACE_END
NET_NAMESPACE_END
NET_NAMESPACE_END
