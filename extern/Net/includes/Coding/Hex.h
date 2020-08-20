#pragma once
#include <Net/Net.h>

#define NET_HEX Net::Coding::HEX

#include <crypto++/includes/hex.h>

#include <Cryption/XOR.h>
#include <assets/assets.h>

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Coding)
BEGIN_CLASS(HEX)
CLASS_PRIVATE
bool encode(CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool encode(CryptoPP::byte**, size_t&);
bool decode(CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool decode(CryptoPP::byte**, size_t&);

CLASS_PUBLIC
bool encodeString(CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool encodeString(CryptoPP::byte**, size_t&);
bool decodeString(CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool decodeString(CryptoPP::byte**, size_t&);
END_CLASS
END_NAMESPACE
END_NAMESPACE