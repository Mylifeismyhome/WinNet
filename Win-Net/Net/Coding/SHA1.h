#pragma once
#include <Net/Net/Net.h>

NET_DSA_BEGIN

#define NET_SHA1 Net::Coding::SHA1

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_CLASS_BEGIN(SHA1)
void ProcessMessageBlock();

void PadMessage();

inline unsigned CircularShift(int, unsigned);

unsigned H[5];                      // Message digest buffers

unsigned Length_Low;                // Message length in bits
unsigned Length_High;               // Message length in bits

unsigned char Message_Block[64];    // 512-bit message blocks
int Message_Block_Index;            // Index into message block array

bool Computed;                      // Is the digest computed?
bool Corrupted;                     // Is the message digest corruped?
NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(SHA1)
NET_CLASS_VDESTRUCTUR(SHA1)

void Reset();

bool Result(unsigned*);

void Input(const unsigned char*, unsigned);
void Input(const char*, unsigned);
void Input(unsigned char);
void Input(char);
SHA1& operator<<(const char*);
SHA1& operator<<(const unsigned char*);
SHA1& operator<<(const char);
SHA1& operator<<(const unsigned char);
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END