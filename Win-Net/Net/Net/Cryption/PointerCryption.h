#pragma once
#undef DISABLE_POINTERCRYPTION
#define CPOINTER PointerCryption

#include <Net/Net/Net.h>
#include <Net/assets/assets.h>

NET_DSA_BEGIN

#define RAND_NUMBER static_cast<uintptr_t>(Net::Random::GetRandSeed())

template <typename T>
class PointerCryptionUniquePointer
{
	T** _pointer;
	uintptr_t _key;

	T* encode(T* pointer) const
	{
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
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
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		return pointer;
	}

	T* encode(const T*& pointer) NOEXPECT
	{
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		return pointer;
	}

	T* decode(T* pointer) const
	{
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
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
