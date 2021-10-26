#pragma once
#include <Net/Net/Net.h>
#include <Net/Cryption/PointerCryption.h>
#include <Net/Cryption/CXOR.hpp>

NET_DSA_BEGIN

#define RUNTIMEXOR Net::Cryption::XOR

#ifdef BUILD_LINUX
#define CASTRING(string) string
#define CWSTRING(string) L##string
#else
#define COMPILETIME_XOR(string) xorstr_(string)
#define WCOMPILETIME_XOR(string) xorstr_(string)
#define CASTRING(string) COMPILETIME_XOR(string)
#define CWSTRING(string) WCOMPILETIME_XOR(L##string)
#endif

#ifdef UNICODE
#define CSTRING(string) CWSTRING(string)
#else
#define CSTRING(string) CASTRING(string)
#endif

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
NET_CLASS_BEGIN(XOR_UNIQUEPOINTER)
CPOINTER<char> buffer;
size_t _length;
bool bFree;

NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(XOR_UNIQUEPOINTER, char*, size_t, bool)
NET_CLASS_DESTRUCTUR(XOR_UNIQUEPOINTER)

char* get() const;
char* data()  const;
char* str() const;
size_t length() const;
size_t size() const;

void free();
NET_CLASS_END

NET_CLASS_BEGIN(XOR)
CPOINTER<char> _buffer;
size_t _length;
CPOINTER<size_t> _Key;

char* encrypt();
char* decrypt() const;
NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(XOR)
NET_CLASS_CONSTRUCTUR(XOR, char*)
void init(char*);
void init(const char*);
size_t size() const;
size_t length() const;
XOR_UNIQUEPOINTER revert(bool = true);
void free();
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END
