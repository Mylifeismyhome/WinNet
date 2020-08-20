#pragma once
#include <Net/Net.h>

#define NET_BASE64 Net::Coding::Base64
#define NET_BASE64V2 Net::Coding::Base64V2

#include <crypto++/includes/base64.h>

#include <Cryption/XOR.h>
#include <assets/assets.h>

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Coding)
BEGIN_CLASS(Base64)
bool encode(const CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool encode(CryptoPP::byte**, size_t&);
bool decode(const CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool decode(CryptoPP::byte**, size_t&);

CLASS_PUBLIC
bool encodeString(const CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool encodeString(CryptoPP::byte**, size_t&);
bool decodeString(const CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool decodeString(CryptoPP::byte**, size_t&);
END_CLASS

BEGIN_CLASS(Base64V2)
bool encode(const CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool encode(CryptoPP::byte**, size_t&);
std::string encode(CryptoPP::byte*, size_t);
bool decode(const CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool decode(CryptoPP::byte**, size_t&);
std::string decode(CryptoPP::byte*, size_t);

CLASS_PUBLIC
bool encodeString(const CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool encodeString(CryptoPP::byte**, size_t&);
std::string encodeString(CryptoPP::byte*, size_t);
bool decodeString(const CryptoPP::byte*, CryptoPP::byte**, size_t&);
bool decodeString(CryptoPP::byte**, size_t&);
std::string decodeString(CryptoPP::byte*, size_t);
END_CLASS
END_NAMESPACE
END_NAMESPACE