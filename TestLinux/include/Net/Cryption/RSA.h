#pragma once
#define NET_RSA Net::Cryption::NETRSA

#include <Net/Net/Net.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <cryptopp/rsa.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/osrng.h>
#include <cryptopp/base64.h>

#include <Net/Cryption/XOR.h>
#include <Net/assets/assets.h>

#include <Net/Coding/BASE64.h>
#include <Net/Coding/Hex.h>

#include <Net/assets/manager/logmanager.h>
NET_DSA_BEGIN

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
NET_CLASS_BEGIN(NETRSA)
RSA* KeyPair;

void generateKeyPair(size_t, int);

// KEYS
RUNTIMEXOR _PublicKey;
RUNTIMEXOR _PrivateKey;
bool Set;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(NETRSA)
NET_CLASS_DESTRUCTUR(NETRSA)

bool generateKeys(size_t, int);

XOR_UNIQUEPOINTER publicKey();
XOR_UNIQUEPOINTER privateKey();

void setPublicKey(char*);
void setPrivateKey(char*);

void deleteKeys();

bool init(const char*, const char*);
bool init(char*, char*);
bool isSet() const { return Set; }

bool encrypt(CryptoPP::byte*&, size_t&);
bool encryptHex(CryptoPP::byte*&, size_t&);
bool encryptBase64(CryptoPP::byte*&, size_t&);
bool decrypt(CryptoPP::byte*&, size_t&);
bool decryptHex(CryptoPP::byte*&, size_t&);
bool decryptBase64(CryptoPP::byte*&, size_t&);
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END
