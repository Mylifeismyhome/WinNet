#pragma once
#undef DISABLE_POINTERCRYPTION
#define CPOINTER PointerCryption

#include <Net/Net/Net.h>
#include <random>

NET_DSA_BEGIN

static int __NET_POINTER_CRYPTION_RNDSEED(int min, int max)
{
	thread_local static std::mt19937 mt(std::random_device{}());
	thread_local static std::uniform_int_distribution<int> pick;
	return pick(mt, decltype(pick)::param_type{ min, max });
}

#define RAND_NUMBER static_cast<uintptr_t>(__NET_POINTER_CRYPTION_RNDSEED(0, INT_MAX))

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
	bool _valid;

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
		_valid = false;
	}
	
	explicit PointerCryption(const T*& pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		_valid = (pointer != nullptr);
	}

	explicit PointerCryption(T*&& pointer) NOEXPECT
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		_valid = (pointer != nullptr);
	}

	PointerCryption& operator=(T* pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		_valid = (pointer != nullptr);
		return *this;
	}

	PointerCryption& operator=(const T*& pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		_valid = (pointer != nullptr);
		return *this;
	}

	void Set(T* pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		_valid = (pointer != nullptr);
	}

	void Set(const T*& pointer)
	{
		_key = RAND_NUMBER;
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		_valid = (pointer != nullptr);
	}

	bool valid() const
	{
		return _valid;
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

		_valid = false;
	}
};
NET_DSA_END
