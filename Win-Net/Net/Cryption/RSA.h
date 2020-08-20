#pragma once
#define NET_RSA Net::Cryption::NETRSA
#define NET_RSA_END CSTRING("-----")

#include <Net/Net.h>

#include <OpenSSL/rsa.h>
#include <OpenSSL/pem.h>
#include <OpenSSL/err.h>

#include <crypto++/includes/rsa.h>
#include <crypto++/includes/cryptlib.h>
#include <crypto++/includes/osrng.h>
#include <crypto++/includes/base64.h>
#include <crypto++/includes/pem.h>

#include <Cryption/XOR.h>
#include <assets/assets.h>

#include <Coding/BASE64.h>
#include <Coding/Hex.h>

NET_DSA_BEGIN

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
NET_CLASS_BEGIN(NETRSA)
// Used to Gen new Keys
CPOINTER<rsa_st> keypair;

// KEYS
RUNTIMEXOR PublicKey;
RUNTIMEXOR PrivateKey;
bool Set;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(NETRSA)
NET_CLASS_DESTRUCTUR(NETRSA)

bool GenerateKeys(size_t, int);
char* CreatePublicKey(size_t, int);
char* CreatePrivateKey(size_t, int);

bool Init(char*, char*);
bool IsSet() const { return Set; }

bool encryptString(CryptoPP::byte**, size_t&);
bool encryptStringHex(CryptoPP::byte**, size_t&);
bool encryptStringBase64(CryptoPP::byte**, size_t&);
bool decryptString(CryptoPP::byte**, size_t&);
bool decryptStringHex(CryptoPP::byte**, size_t&);
bool decryptStringBase64(CryptoPP::byte**, size_t&);
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END