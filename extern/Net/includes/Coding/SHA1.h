#pragma once
#include <Net/Net.h>

#define NET_SHA1 Net::Coding::SHA1

BEGIN_NAMESPACE(Net)
BEGIN_NAMESPACE(Coding)
BEGIN_CLASS(SHA1)
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
CLASS_PUBLIC
CLASS_CONSTRUCTUR(SHA1)
CLASS_VDESTRUCTUR(SHA1)

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
END_CLASS
END_NAMESPACE
END_NAMESPACE
