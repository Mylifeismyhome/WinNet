#pragma once
#include <Net/Net.h>

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

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Cryption)
BEGIN_CLASS(AES)
/* STORE KEYS */
RUNTIMEXOR* Key;
RUNTIMEXOR* IV;
u_short KeyLength;

/* ACTUALL ENC/DEC FUNC */
bool encrypt(CryptoPP::byte*, const size_t, const char*, const char*) const;
bool decrypt(CryptoPP::byte*, const size_t, const char*, const char*) const;

CLASS_PUBLIC
CLASS_CONSTRUCTUR(AES)
CLASS_DESTRUCTUR(AES)

/* FIRST INIT AES */
bool Init(char*, char*, const u_short = CryptoPP::AES::DEFAULT_KEYLENGTH);

bool encryptString(CryptoPP::byte*, const size_t) const;
bool encryptStringHex(CryptoPP::byte*, CryptoPP::byte**, size_t&) const;
bool encryptStringHex(CryptoPP::byte**, size_t&) const;
bool encryptStringBase64(CryptoPP::byte**, size_t&) const;
bool encryptStringBase64(CryptoPP::byte*, CryptoPP::byte**, size_t&) const;
bool decryptString(CryptoPP::byte*, const size_t) const;
bool decryptStringHex(CryptoPP::byte*, CryptoPP::byte**, size_t&) const;
bool decryptStringHex(CryptoPP::byte**, size_t&) const;
bool decryptStringBase64(CryptoPP::byte**, size_t&) const;
bool decryptStringBase64(CryptoPP::byte*, CryptoPP::byte**, size_t&) const;
END_CLASS
END_NAMESPACE
END_NAMESPACE