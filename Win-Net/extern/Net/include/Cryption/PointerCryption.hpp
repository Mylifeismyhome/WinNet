#pragma once
#define CPOINTER PointerCryption

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
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)PointerCryptionCompileTimeXORKey);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		pointer = (T*)EncodePointer(pointer);
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
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)PointerCryptionCompileTimeXORKey);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		pointer = (T*)EncodePointer(pointer);
		return pointer;
	}

	T* encode(const T*& pointer) noexcept
	{
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)PointerCryptionCompileTimeXORKey);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		pointer = (T*)EncodePointer(pointer);
		return pointer;
	}

	T* decode(T* pointer) const
	{
		pointer = (T*)DecodePointer(pointer);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)_key);
		pointer = (T*)((uintptr_t)pointer ^ (uintptr_t)PointerCryptionCompileTimeXORKey);
		return pointer;
	}

public:
	PointerCryption()
	{
		//_key = Net::Random::GetRandSeed();
		_pointer = nullptr;
	}
	
	explicit PointerCryption(const T*& pointer)
	{
		//_key = Net::Random::GetRandSeed();
		_pointer = encode(pointer);
	}

	explicit PointerCryption(T*&& pointer) noexcept
	{
		//_key = Net::Random::GetRandSeed();
		_pointer = encode(pointer);
	}

	~PointerCryption()
	{
		free();
	}

	PointerCryption& operator=(const T*& pointer)
	{
		free();
		//_key = Net::Random::GetRandSeed();
		_pointer = encode(pointer);
		return *this;
	}

	PointerCryption& operator=(T*&& pointer) noexcept
	{
		free();
		//_key = Net::Random::GetRandSeed();
		_pointer = encode(pointer);
		return *this;
	}

	bool valid() const
	{
		return _pointer == nullptr ? false : get() != nullptr;
	}

	T* get() const
	{
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