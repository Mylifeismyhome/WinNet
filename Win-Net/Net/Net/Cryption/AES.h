#pragma once
#include <Net/Net/Net.h>

#define NET_AES Net::Cryption::AES

#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>

#include <Net/Cryption/XOR.h>
#include <Net/Coding/Hex.h>
#include <Net/Coding/BASE64.h>
#include <Net/assets/assets.h>

NET_DSA_BEGIN

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
NET_CLASS_BEGIN(AES)
/* STORE KEYS */
RUNTIMEXOR Key;
RUNTIMEXOR IV;
size_t KeyLength;

/* ACTUALL ENC/DEC FUNC */
bool encrypt(CryptoPP::byte*, const size_t, const char*, const char*) const;
bool decrypt(CryptoPP::byte*, const size_t, const char*, const char*) const;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(AES)
NET_CLASS_DESTRUCTUR(AES)

/* FIRST INIT AES */
bool init(const char*, const char*, size_t = CryptoPP::AES::DEFAULT_KEYLENGTH);

bool encrypt(CryptoPP::byte*, size_t);
bool encryptHex(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
bool encryptHex(CryptoPP::byte*&, size_t&);
bool encryptBase64(CryptoPP::byte*&, size_t&);
bool encryptBase64(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
bool decrypt(CryptoPP::byte*, size_t);
bool decryptHex(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
bool decryptHex(CryptoPP::byte*&, size_t&);
bool decryptBase64(CryptoPP::byte*&, size_t&);
bool decryptBase64(CryptoPP::byte*, CryptoPP::byte*&, size_t&);
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END
