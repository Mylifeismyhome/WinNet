#pragma once
#undef DISABLE_POINTERCRYPTION
#define CPOINTER PointerCryption

#include <random>
#include <Net/Security/NET_Kernel32.h>

NET_DSA_BEGIN

#ifndef VS13
template <typename TYPE>
TYPE GetRandNumber()
{
	thread_local static std::mt19937 mt(std::random_device{}());
	thread_local static std::uniform_int_distribution<TYPE> pick;
	return pick(mt);
}

static uintptr_t GetRandSeed()
{
	return GetRandNumber<uintptr_t>();
}
#endif

#ifdef VS13
#define RAND_NUMBER NULL
#else
#define RAND_NUMBER GetRandSeed()
#endif

template <typename T>
class PointerCryptionUniquePointer
{
	T** _pointer;
	uintptr_t _key;

	T* encode(T* pointer) const
	{
#ifndef DISABLE_POINTERCRYPTION
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
#ifndef DISABLE_MS_POINTERCRYPTION
		pointer = (T*)NET_Kernel32::EncodePointer(pointer);
#endif
#endif
		return pointer;
	}

public:
	explicit PointerCryptionUniquePointer(T** pointer, const uintptr_t key)
	{
		_key = key;
		_pointer = pointer;
	}

	~PointerCryptionUniquePointer()
	{
		if (*_pointer)
			*_pointer = encode(*_pointer);
	}

	T*& get()
	{
		return *_pointer;
	}
};

template <typename T>
class PointerCryption
{
	T* _pointer;
	uintptr_t _key;

	T* encode(T* pointer) NOEXPECT
	{
#ifndef DISABLE_POINTERCRYPTION
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
#ifndef DISABLE_MS_POINTERCRYPTION
		pointer = (T*)NET_Kernel32::EncodePointer(pointer);
#endif
#endif
		return pointer;
	}

	T* encode(const T*& pointer) NOEXPECT
	{
#ifndef DISABLE_POINTERCRYPTION
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
#ifndef DISABLE_MS_POINTERCRYPTION
		pointer = (T*)NET_Kernel32::EncodePointer(pointer);
#endif
#endif
		return pointer;
	}

	T* decode(T* pointer) const
	{
#ifndef DISABLE_POINTERCRYPTION
#ifndef DISABLE_MS_POINTERCRYPTION
		pointer = (T*)NET_Kernel32::DecodePointer(pointer);
#endif
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
#endif
		return pointer;
	}

public:
	PointerCryption()
	{
		_key = RAND_NUMBER;
		_pointer = nullptr;
	}
	
	explicit PointerCryption(const T*& pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
	}

	explicit PointerCryption(T*&& pointer) NOEXPECT
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
	}

	PointerCryption& operator=(T* pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		return *this;
	}

	PointerCryption& operator=(const T*& pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		return *this;
	}

	void Set(T* pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
	}

	void Set(const T*& pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
	}

	bool valid() const
	{
		return _pointer == nullptr ? false : get() != nullptr;
	}

	T* get() const
	{
		if (!_pointer)
			return nullptr;
		
		return decode(_pointer);
	}

	PointerCryptionUniquePointer<T> reference()
	{
		_pointer = decode(_pointer);
		return PointerCryptionUniquePointer<T>(&_pointer, _key);
	}

	void free()
	{
		if (!valid())
			return;

		_pointer = decode(_pointer);
		FREE(_pointer);
	}
};
NET_DSA_END