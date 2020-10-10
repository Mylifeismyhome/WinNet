#pragma once
#define DISABLE_POINTERCRYPTION
#define CPOINTER PointerCryption

#include <random>

NET_DSA_BEGIN

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

constexpr static auto Time2String(const char* str, const int offset) {
	return static_cast<std::uint32_t>(str[offset] - '0') * 10 +
		static_cast<std::uint32_t>(str[offset + 1] - '0');
}

constexpr static auto RndSeed() {
	return Time2String(__TIME__, 0) * 60 * 60 + Time2String(__TIME__, 3) * 60 + Time2String(__TIME__, 6);
}

constexpr uintptr_t PointerCryptionCompileTimeXORKey = {
	 RndSeed()
};

template <typename T>
class PointerCryptionUniquePointer
{
	T** _pointer;
	uintptr_t _key;

	T* encode(T* pointer) const
	{
#ifndef DISABLE_POINTERCRYPTION
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)PointerCryptionCompileTimeXORKey);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		pointer = (T*)EncodePointer(pointer);
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
		if(*_pointer)
			*_pointer = encode(*_pointer);
	}

	T** get()
	{
		return _pointer;
	}
};

template <typename T>
class PointerCryption
{
	T* _pointer;
	uintptr_t _key;

	T* encode(T* pointer) noexcept
	{
#ifndef DISABLE_POINTERCRYPTION
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)PointerCryptionCompileTimeXORKey);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		pointer = (T*)EncodePointer(pointer);
#endif
		return pointer;
	}

	T* encode(const T*& pointer) noexcept
	{
#ifndef DISABLE_POINTERCRYPTION
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)PointerCryptionCompileTimeXORKey);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		pointer = (T*)EncodePointer(pointer);
#endif
		return pointer;
	}

	T* decode(T* pointer) const
	{
#ifndef DISABLE_POINTERCRYPTION
		pointer = (T*)DecodePointer(pointer);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)PointerCryptionCompileTimeXORKey);
#endif
		return pointer;
	}

public:
	PointerCryption()
	{
		_key = GetRandSeed();
		_pointer = nullptr;
	}
	
	explicit PointerCryption(const T*& pointer)
	{
		_key = GetRandSeed();
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
	}

	explicit PointerCryption(T*&& pointer) noexcept
	{
		_key = GetRandSeed();
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
	}

	PointerCryption& operator=(T* pointer)
	{
		_key = GetRandSeed();
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		return *this;
	}

	PointerCryption& operator=(const T*& pointer)
	{
		_key = GetRandSeed();
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
		return *this;
	}

	void Set(T* pointer)
	{
		_key = GetRandSeed();
		_pointer = pointer == nullptr ? nullptr : encode(pointer);
	}

	void Set(const T*& pointer)
	{
		_key = GetRandSeed();
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