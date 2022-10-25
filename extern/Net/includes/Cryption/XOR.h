#pragma once
#include <Net/Net.h>
#include <time.h>

#define RUNTIMEXOR Net::Cryption::XOR
#define XORREVERT(x) x == nullptr ? "[ERROR] - Invalid Object" : x->Revert()
#define FREE_XOR(x) \
if(x) \
{ \
delete x; \
x = NULL; \
}

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Cryption)
BEGIN_CLASS(XOR)
char* uncrypted;
char* encrypted;
size_t length;

__int64* Key;

const char* Encrypt();
const char* Decrypt();
CLASS_PUBLIC
CLASS_CONSTRUCTUR(XOR, char*)
CLASS_DESTRUCTUR(XOR)
const char* Revert();
END_CLASS
END_NAMESPACE
END_NAMESPACE