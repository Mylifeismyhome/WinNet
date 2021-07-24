#pragma once
#include <Net/Net/Net.h>
#include <Net/Cryption/PointerCryption.h>

NET_DSA_BEGIN

#define RUNTIMEXOR Net::Cryption::XOR
#ifndef VS13
#define COMPILETIME_XOR(string) []{ CONSTEXPR size_t COMPILETIME_XORKEY = 0 + Net::Cryption::COMPILETIME_XOR_LCG(10) % (0xFF - 0 + 1); CONSTEXPR Net::Cryption::CXOR<(sizeof(string)/sizeof(char)), char, COMPILETIME_XORKEY> expr(string); return expr; }().decrypt()
#define WCOMPILETIME_XOR(string) []{ CONSTEXPR size_t COMPILETIME_XORKEY = 0 + Net::Cryption::COMPILETIME_XOR_LCG(10) % (0xFF - 0 + 1); CONSTEXPR Net::Cryption::CXOR<(sizeof(string)/sizeof(wchar_t)), wchar_t, COMPILETIME_XORKEY> expr(string); return expr; }().decrypt()
#define CASTRING(string) COMPILETIME_XOR(string)
#define CWSTRING(string) WCOMPILETIME_XOR(L##string)
#endif

#ifdef VS13
#define CSTRING(string) string
#define CWSTRING(string) L##string
#define CASTRING(string) string
#else
#ifdef UNICODE
#define CSTRING(string) CWSTRING(string)
#else
#define CSTRING(string) CASTRING(string)
#endif
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

char* Encrypt();
char* Decrypt() const;
NET_CLASS_PUBLIC
NET_CLASS_CONSTRUCTUR(XOR)
NET_CLASS_CONSTRUCTUR(XOR, char*)
void Init(char*);
size_t size() const;
size_t length() const;
XOR_UNIQUEPOINTER Revert(bool = true);
void free();
NET_CLASS_END

#ifndef VS13
CONSTEXPR size_t COMPILETIME_XOR_LCG(const unsigned rounds) {
	return SIZE_MAX + SIZE_MAX * (rounds > 0 ? COMPILETIME_XOR_LCG(rounds - 1) : SIZE_MAX) % 0xFFFFFFFF;
}
#endif

template <unsigned size, typename Char, const size_t Key>
class CXOR {
public:
	const unsigned _numchars = (size - 1);
	Char _string[size];

	explicit CONSTEXPR CXOR(const Char* string) : _string{}
	{
		for (auto i = 0u; i < size; ++i)
			_string[i] = string[i] ^ (static_cast<Char>(Key) + i);
	}

	const Char* decrypt() const
	{
		Char* string = const_cast<Char*>(_string);
		for (unsigned t = 0; t < _numchars; t++) {
			string[t] = string[t] ^ (static_cast<Char>(Key) + t);
		}
		string[_numchars] = '\0';
		return string;
	}
};
NET_NAMESPACE_END
NET_NAMESPACE_END

NET_DSA_END