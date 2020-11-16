#pragma once
#include <Net/Net/Net.h>

#define NET_AES Net::Cryption::AES

#include <crypto++/includes/modes.h>
#include <crypto++/includes/aes.h>
#include <crypto++/includes/filters.h>
#include <crypto++/includes/base64.h>
#include <crypto++/includes/hex.h>

#include <Cryption/XOR.h>
#include <Coding/HEX.h>
#include <Coding/BASE64.h>
#include <assets/assets.h>

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
bool Init(char*, char*, size_t = CryptoPP::AES::DEFAULT_KEYLENGTH);

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