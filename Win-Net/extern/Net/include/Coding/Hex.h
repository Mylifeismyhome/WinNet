#pragma once
#define NET_HEX Net::Coding::HEX

#include <Net/Net/Net.h>
#include <crypto++/includes/hex.h>
#include <Cryption/XOR.h>
#include <assets/assets.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_DSA_BEGIN
NET_CLASS_BEGIN(HEX)
NET_CLASS_PUBLIC
bool encode(CryptoPP::byte*, CryptoPP::byte*&, size_t&)  const;
bool encode(CryptoPP::byte*&, size_t&) const;
bool decode(CryptoPP::byte*, CryptoPP::byte*&, size_t&) const;
bool decode(CryptoPP::byte*&, size_t&) const;
NET_CLASS_END
NET_DSA_END
NET_NAMESPACE_END
NET_NAMESPACE_END