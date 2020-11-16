#pragma once
#define NET_BASE64_PATTERN CSTRING("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/")
#define NET_BASE64 const Net::Coding::Base64

#include <Net/Net/Net.h>
#include <crypto++/includes/base64.h>
#include <Net/Cryption/XOR.h>
#include <cstdio>

NET_DSA_BEGIN

/* Base64 by NibbleAndAHalf | https://github.com/superwills/NibbleAndAHalf/blob/master/NibbleAndAHalf/ */

// maps A=>0,B=>1..
const static unsigned char unb64[] = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //10 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //20 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //30 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //40 
  0,   0,   0,  62,   0,   0,   0,  63,  52,  53, //50 
 54,  55,  56,  57,  58,  59,  60,  61,   0,   0, //60 
  0,   0,   0,   0,   0,   0,   1,   2,   3,   4, //70 
  5,   6,   7,   8,   9,  10,  11,  12,  13,  14, //80 
 15,  16,  17,  18,  19,  20,  21,  22,  23,  24, //90 
 25,   0,   0,   0,   0,   0,   0,  26,  27,  28, //100 
 29,  30,  31,  32,  33,  34,  35,  36,  37,  38, //110 
 39,  40,  41,  42,  43,  44,  45,  46,  47,  48, //120 
 49,  50,  51,   0,   0,   0,   0,   0,   0,   0, //130 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //140 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //150 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //160 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //170 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //180 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //190 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //200 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //210 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //220 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //230 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //240 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //250 
  0,   0,   0,   0,   0,   0,
}; // This array has 256 elements

// Converts binary data of length=len to base64 characters.
// Length of the resultant string is stored in flen
// (you must pass pointer flen).
inline unsigned char* Base64_Encode(const unsigned char* data, const size_t len, size_t* flen, const bool preventDelete = false)
{
	size_t rc = 0;
	size_t byteNo = 0;

	const auto modulusLen = len % 3;
	const auto pad = ((modulusLen & 1) << 1) + ((modulusLen & 2) >> 1); // 2 gives 1 and 1 gives 2, but 0 gives 0.

	*flen = 4 * (len + pad) / 3;

	const auto res = new unsigned char[*flen + 1];
	for (byteNo = 0; byteNo <= len - 3; byteNo += 3)
	{
		const auto BYTE0 = data[byteNo];
		const auto BYTE1 = data[byteNo + 1];
		const auto BYTE2 = data[byteNo + 2];
		res[rc++] = NET_BASE64_PATTERN[BYTE0 >> 2];
		res[rc++] = NET_BASE64_PATTERN[((0x3 & BYTE0) << 4) + (BYTE1 >> 4)];
		res[rc++] = NET_BASE64_PATTERN[((0x0f & BYTE1) << 2) + (BYTE2 >> 6)];
		res[rc++] = NET_BASE64_PATTERN[0x3f & BYTE2];
	}

	if (pad == 2)
	{
		res[rc++] = NET_BASE64_PATTERN[data[byteNo] >> 2];
		res[rc++] = NET_BASE64_PATTERN[(0x3 & data[byteNo]) << 4];
		res[rc++] = '=';
		res[rc++] = '=';
	}
	else if (pad == 1)
	{
		res[rc++] = NET_BASE64_PATTERN[data[byteNo] >> 2];
		res[rc++] = NET_BASE64_PATTERN[((0x3 & data[byteNo]) << 4) + (data[byteNo + 1] >> 4)];
		res[rc++] = NET_BASE64_PATTERN[(0x0f & data[byteNo + 1]) << 2];
		res[rc++] = '=';
	}

	if(!preventDelete)
	{
		delete[] data;
		data = nullptr;
	}
	
	res[rc] = 0;
	return res;
}

inline unsigned char* Base64_Decode(const unsigned char* ascii, const size_t len, size_t* flen, const bool preventDelete = false)
{
	size_t cb = 0;
	size_t charNo;
	auto pad = 0;

	if (len < 2)
	{
		// 2 accesses below would be OOB.
		// catch empty string, return NULL as result.
		puts(CSTRING("ERROR: You passed an invalid base64 string (too short). You get NULL back."));
		*flen = 0;
		return nullptr;
	}
	if (ascii[len - 1] == '=')  ++pad;
	if (ascii[len - 2] == '=')  ++pad;

	*flen = 3 * len / 4 - pad;
	const auto bin = new unsigned char[*flen + 1];
	if (!bin)
	{
		puts(CSTRING("ERROR: unbase64 could not allocate enough memory."));
		puts(CSTRING("I must stop because I could not get enough"));
		return nullptr;
	}

	for (charNo = 0; charNo <= len - 4 - pad; charNo += 4)
	{
		const auto A = unb64[ascii[charNo]];
		const auto B = unb64[ascii[charNo + 1]];
		const auto C = unb64[ascii[charNo + 2]];
		const auto D = unb64[ascii[charNo + 3]];

		bin[cb++] = (A << 2) | (B >> 4);
		bin[cb++] = (B << 4) | (C >> 2);
		bin[cb++] = (C << 6) | (D);
	}

	if (pad == 1)
	{
		const auto A = unb64[ascii[charNo]];
		const auto B = unb64[ascii[charNo + 1]];
		const auto C = unb64[ascii[charNo + 2]];

		bin[cb++] = (A << 2) | (B >> 4);
		bin[cb++] = (B << 4) | (C >> 2);
	}
	else if (pad == 2)
	{
		const auto A = unb64[ascii[charNo]];
		const auto B = unb64[ascii[charNo + 1]];

		bin[cb++] = (A << 2) | (B >> 4);
	}

	if(!preventDelete)
	{
		delete[] ascii;
		ascii = nullptr;
	}

	bin[cb] = 0;
	return bin;
}

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
NET_CLASS_BEGIN(Base64)
NET_CLASS_PUBLIC
bool encode(BYTE*, BYTE*&, size_t&) const;
bool encode(BYTE*&, size_t&) const;
bool decode(BYTE*, BYTE*&, size_t&) const;
bool decode(BYTE*&, size_t&)const ;
NET_CLASS_END
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END