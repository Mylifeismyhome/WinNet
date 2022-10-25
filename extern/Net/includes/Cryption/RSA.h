#pragma once
#pragma once
#include <Net/Net.h>

#define NET_RSA Net::Cryption::NETRSA

#include <OpenSSL/include/openssl/rsa.h>
#include <OpenSSL/include/openssl/pem.h>
#include <OpenSSL/include/openssl/err.h>

#include <crypto++/includes/rsa.h>
#include <crypto++/includes/cryptlib.h>
#include <crypto++/includes/osrng.h>
#include <crypto++/includes/base64.h>
#include <crypto++/includes/pem.h>

#include <Cryption/XOR.h>
#include <assets/assets.h>

#include <Coding/BASE64.h>
#include <Coding/Hex.h>

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Cryption)
BEGIN_CLASS(NETRSA)
// Used to Gen new Keys
rsa_st* keypair;

// KEYS
RUNTIMEXOR* PublicKey;
RUNTIMEXOR* PrivateKey;
bool Set;

CLASS_PUBLIC
CLASS_CONSTRUCTUR(NETRSA)
CLASS_DESTRUCTUR(NETRSA)

bool GenerateKeys(size_t, int);
char* CreatePublicKey(size_t, int);
char* CreatePrivateKey(size_t, int);

bool Init(char*, char*);
bool IsSet() { return Set; }

bool encryptString(CryptoPP::byte**, size_t&) const;
bool encryptStringHex(CryptoPP::byte**, size_t&) const;
bool encryptStringBase64(CryptoPP::byte**, size_t&) const;
bool decryptString(CryptoPP::byte**, size_t&) const;
bool decryptStringHex(CryptoPP::byte**, size_t&) const;
bool decryptStringBase64(CryptoPP::byte**, size_t&) const;
END_CLASS
END_NAMESPACE
END_NAMESPACE