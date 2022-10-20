#include <Net/Net/NetString.h>

NET_IGNORE_CONVERSION_NULL
namespace Net
{
	namespace Pointer
	{
		UniquePointer::UniquePointer(void* in)
		{
			_pointer = in;
		}

		UniquePointer::~UniquePointer()
		{
			FREE(_pointer);
		}
	}

	String::String()
	{
		_string = RUNTIMEXOR();
		_free_size = INVALID_SIZE;
	}

	String::String(const char in)
	{
		std::vector<char> str(2);
		str[0] = in;
		str[1] = '\0';

		_string = RUNTIMEXOR(reinterpret_cast<const char*>(str.data()));
		_free_size = INVALID_SIZE;
	}

	String::String(const char* in, ...)
	{
		va_list vaArgs;
		va_start(vaArgs, in);
		const size_t size = std::vsnprintf(nullptr, 0, in, vaArgs);
		va_end(vaArgs);

		va_start(vaArgs, in);
		std::vector<char> str(size + 1);
		std::vsnprintf(str.data(), str.size(), in, vaArgs);
		va_end(vaArgs);

		_string = RUNTIMEXOR(reinterpret_cast<const char*>(str.data()));
		_free_size = INVALID_SIZE;
	}

	String::String(char* str)
	{
		/*
		* instead of allocating new space and copy this string
		* we just move it's pointer into ours
		*/
		_string.free();
		_string = RUNTIMEXOR(str);
		_free_size = INVALID_SIZE;
	}

	String::String(String& in)
	{
		copy(in);
	}

	String::String(String&& in) NOEXPECT
	{
		move((String&&)in);
	}

	void String::Construct(const char in)
	{
		std::vector<char> str(2);
		str[0] = in;
		str[1] = '\0';

		this->_string = RUNTIMEXOR(reinterpret_cast<const char*>(str.data()));
		this->_free_size = INVALID_SIZE;
	}

	void String::Construct(const char* in, ...)
	{
		va_list vaArgs;
		va_start(vaArgs, in);
		const size_t size = std::vsnprintf(nullptr, 0, in, vaArgs);
		va_end(vaArgs);

		va_start(vaArgs, in);
		std::vector<char> str(size + 1);
		std::vsnprintf(str.data(), str.size(), in, vaArgs);
		va_end(vaArgs);

		this->_string = RUNTIMEXOR(reinterpret_cast<const char*>(str.data()));
		this->_free_size = INVALID_SIZE;
	}

	void String::copy(String& in)
	{
		this->_string.free();

		auto ref = in.get();
		auto pBuffer = ref.get();
		this->_string = RUNTIMEXOR(reinterpret_cast<const char*>(pBuffer));
		this->_free_size = INVALID_SIZE;
	}

	void String::move(String&& in)
	{
		_string = in._string;
		_free_size = in._free_size;

		/*
		* set _string to nullptr
		* we moved the pointer to a new object
		*/
		in._string = RUNTIMEXOR();
		in._free_size = INVALID_SIZE;
	}

	String::~String()
	{
		_string.free();
	}

	size_t String::size() const
	{
		return _string.size();
	}

	size_t String::length() const
	{
		return _string.length();
	}

	void String::set(const char in, ...)
	{
		std::vector<char> str(2);
		str[0] = in;
		str[1] = '\0';

		this->_string.free();
		this->_string = RUNTIMEXOR(reinterpret_cast<const char*>(str.data()));
		this->_free_size = INVALID_SIZE;
	}

	void String::set(const char* in, ...)
	{
		va_list vaArgs;

#ifdef BUILD_LINUX
		va_start(vaArgs, in);
		const size_t size = std::vsnprintf(nullptr, 0, in, vaArgs);
		va_end(vaArgs);

		va_start(vaArgs, in);
		std::vector<char> str(size + 1);
		std::vsnprintf(str.data(), str.size(), in, vaArgs);
		va_end(vaArgs);
#else
		va_start(vaArgs, in);
		const size_t size = std::vsnprintf(nullptr, 0, in, vaArgs);
		std::vector<char> str(size + 1);
		std::vsnprintf(str.data(), str.size(), in, vaArgs);
		va_end(vaArgs);
#endif

		this->_string.free();
		this->_string = RUNTIMEXOR(reinterpret_cast<const char*>(str.data()));
		this->_free_size = INVALID_SIZE;
	}

	void String::set(String& in, ...)
	{
		auto ref = in.get();
		auto ptr = ref.get();

		/*
		* cast it to const char*
		* so we will perform a copy
		*/
		this->_string.free();
		this->_string = RUNTIMEXOR(reinterpret_cast<const char*>(ptr));
		this->_free_size = INVALID_SIZE;
	}

	void String::append(const char in)
	{
		if (size() == INVALID_SIZE || size() == 0)
		{
			Construct(in);
			return;
		}
		
		if (_free_size >= 1)
		{
			/*
			* use _free_size to fill up the free space in our current buffer
			*/
			auto prevLen = size();
			_string.set_size(size() + 1);
			_string.set(prevLen, in);
			_string.set(size(), '\0');
			_free_size -= 1;
		}
		else
		{
			size_t newLen = _string.size() + 1;
			NET_CPOINTER<byte> data(ALLOC<byte>(newLen + 1));
			memcpy(&data.get()[0], _string.revert().get(), _string.size());
			data.get()[_string.size()] = in;
			data.get()[newLen] = '\0';

			_string.free();
			_string = RUNTIMEXOR(reinterpret_cast<char*>(data.get()));
		}
	}

	void Net::String::append(char* buffer)
	{
		if (size() == INVALID_SIZE || size() == 0)
		{
			Construct(buffer);
			return;
		}

		auto buffer_len = strlen(buffer);
		if (buffer_len > _free_size)
		{
			_free_size = INVALID_SIZE;

			size_t newSize = _string.size() + buffer_len;
			NET_CPOINTER<byte> data(ALLOC<byte>(newSize + 1));
			memcpy(data.get(), _string.revert().get(), _string.size());
			memcpy(&data.get()[_string.size()], buffer, buffer_len);
			data.get()[newSize] = '\0';

			_string.free();
			_string = RUNTIMEXOR(reinterpret_cast<const char*>(data.get()));

			return;
		}

		/*
		* use _free_size to fill up the free space in our current buffer
		*/
		auto prevLen = size();
		_string.set_size(size() + buffer_len);
		for (size_t i = prevLen, j = 0; j < buffer_len; ++j)
		{
			_string.set(i + j, buffer[j]);
		}
		_string.set(size(), '\0');
		_free_size -= buffer_len;
	}

	void String::append(const char* in, ...)
	{
		if (size() == INVALID_SIZE || size() == 0)
		{
			Construct(in);
			return;
		}

		va_list vaArgs;

#ifdef BUILD_LINUX
		va_start(vaArgs, in);
		const size_t size = std::vsnprintf(nullptr, 0, in, vaArgs);
		va_end(vaArgs);

		va_start(vaArgs, in);
		std::vector<char> str(size + 1);
		std::vsnprintf(str.data(), str.size(), in, vaArgs);
		va_end(vaArgs);
#else
		va_start(vaArgs, in);
		const size_t size = std::vsnprintf(nullptr, 0, in, vaArgs);
		std::vector<char> str(size + 1);
		std::vsnprintf(str.data(), str.size(), in, vaArgs);
		va_end(vaArgs);
#endif

		this->append(str.data());
	}

	void String::append(String& in, ...)
	{
		if (size() == INVALID_SIZE || size() == 0)
		{
			copy(in);
			return;
		}

		auto ref = in.get();
		auto ptr = ref.get();
		this->append(ptr);
	}

	Net::Cryption::XOR_UNIQUEPOINTER String::str()
	{
		if (size() == INVALID_SIZE)
			return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

		return _string.revert();
	}

	Net::Cryption::XOR_UNIQUEPOINTER String::cstr()
	{
		if (size() == INVALID_SIZE)
			return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

		return _string.revert();
	}

	Net::Cryption::XOR_UNIQUEPOINTER String::get()
	{
		if (size() == INVALID_SIZE)
			return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

		return _string.revert();
	}

	Net::Cryption::XOR_UNIQUEPOINTER String::revert()
	{
		if (size() == INVALID_SIZE)
			return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

		return _string.revert();
	}

	Net::Cryption::XOR_UNIQUEPOINTER String::data()
	{
		if (size() == INVALID_SIZE)
			return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

		return _string.revert();
	}

	void String::clear()
	{
		_string.free();
	}

	bool String::empty()
	{
		if (size() == INVALID_SIZE || size() == 0)
			return true;

		return (this->compare(CSTRING("")));
	}

	char String::at(const size_t i)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return '\0';

		if (i > size())
			return '\0';

		return this->operator[](i);
	}

	char* String::substr(size_t length)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return nullptr;

		if (length > size())
			length = size();

		const auto sub = ALLOC<char>(length + 1);
		memset(sub, NULL, length);
		for (size_t i = 0, j = 0; i < length; ++i, ++j)
		{
			const auto tmp = at(i);
			memcpy(&sub[j], &tmp, 1);
		}
		sub[length] = '\0';

		return sub;
	}

	char* String::substr(const size_t start, size_t length)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return nullptr;

		if (length > size())
			length = size();

		if (start + length > size())
			length = size() - start;

		const auto sub = ALLOC<char>(length + 1);
		memset(sub, NULL, length);
		for (size_t i = start, j = 0; i < (start + length); ++i, ++j)
		{
			const auto tmp = at(i);
			memcpy(&sub[j], &tmp, 1);
		}
		sub[length] = '\0';

		return sub;
	}

	size_t String::find(const char c, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return NET_STRING_NOT_FOUND;

		for (size_t i = 0; i < size(); ++i)
		{
			const auto tmp = type & NOT_CASE_SENS ? (char)tolower((int)at(i)) : at(i);
			const auto comp = type & NOT_CASE_SENS ? (char)tolower((int)c) : c;
			if(tmp == comp)
				return i;
		}

		return NET_STRING_NOT_FOUND;
	}

	size_t String::find(const char* pattern, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return NET_STRING_NOT_FOUND;

		const auto patternLen = strlen(pattern);

		size_t it = 0;
		for (size_t i = 0; i < size(); ++i)
		{
			const auto tmp = (type & Net::String::type::NOT_CASE_SENS) ? (char)tolower((int)at(i)) : at(i);
			const auto comp = (type & Net::String::type::NOT_CASE_SENS) ? (char)tolower((int)pattern[it]) : pattern[it];
			if(tmp == comp)
			{
				it++;
				if (it == patternLen)
				{
					return i - it + 1;
				}
			}
			else
			{
				it = 0;
			}
		}

		return NET_STRING_NOT_FOUND;
	}

	size_t String::find(const size_t start, char character, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return NET_STRING_NOT_FOUND;

		if (start > size())
			return NET_STRING_NOT_FOUND;

		if (type & NOT_CASE_SENS)
		{
			character = (char)tolower((int)character);
		}

		for (auto i = start; i < size(); ++i)
		{
			const auto tmp = (char)tolower((int)at(i));
			if (tmp == character)
				return i;
		}

		return NET_STRING_NOT_FOUND;
	}

	size_t String::find(const size_t start, const char* pattern, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return NET_STRING_NOT_FOUND;

		if (start > size())
			return NET_STRING_NOT_FOUND;

		const auto patternLen = strlen(pattern);

		size_t it = 0;
		for (auto i = start; i < size(); ++i)
		{
			const auto tmp = (type & Net::String::type::NOT_CASE_SENS) ? (char)tolower((int)at(i)) : at(i);
			const auto comp = (type & Net::String::type::NOT_CASE_SENS) ? (char)tolower((int)pattern[it]) : pattern[it];
			if (tmp == comp)
			{
				it++;
				if (it == patternLen)
				{
					return i - it + 1;
				}
			}
			else
			{
				it = 0;
			}
		}

		return NET_STRING_NOT_FOUND;
	}

	std::vector<size_t> String::findAll(const char c, const char type)
	{
		std::vector<size_t> tmp;

		if (size() == INVALID_SIZE || size() == 0)
			return tmp;

		for (size_t i = 0; i < size(); ++i)
		{
			const auto res = find(i, c, type);
			if (res != NET_STRING_NOT_FOUND)
				tmp.emplace_back(res);
		}

		return tmp;
	}

	std::vector<size_t> String::findAll(const char* pattern, const char type)
	{
		std::vector<size_t> tmp;

		if (size() == INVALID_SIZE || size() == 0)
			return tmp;

		for (size_t i = 0; i < size(); ++i)
		{
			const auto res = find(i, pattern, type);
			if (res != NET_STRING_NOT_FOUND)
				tmp.emplace_back(res);
		}

		return tmp;
	}

	std::vector<size_t> String::findAll(size_t start, const char c, const char type)
	{
		std::vector<size_t> tmp;

		if (size() == INVALID_SIZE || size() == 0)
			return tmp;

		if (start > size())
			return tmp;

		for (size_t i = start; i < size(); ++i)
		{
			const auto res = find(i, c, type);
			if (res != NET_STRING_NOT_FOUND)
				tmp.emplace_back(res);
		}

		return tmp;
	}

	std::vector<size_t> String::findAll(size_t start, const char* pattern, const char type)
	{
		std::vector<size_t> tmp;

		if (size() == INVALID_SIZE || size() == 0)
			return tmp;

		if (start > size())
			return tmp;

		for (size_t i = start; i < size(); ++i)
		{
			const auto res = find(i, pattern, type);
			if (res != NET_STRING_NOT_FOUND)
				tmp.emplace_back(res);
		}

		return tmp;
	}

	size_t String::findLastOf(const char c, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return NET_STRING_NOT_FOUND;

		for (size_t i = size() - 1; i > 0; --i)
		{
			const auto tmp = type & NOT_CASE_SENS ? (char)tolower((int)at(i)) : at(i);
			const auto comp = type & NOT_CASE_SENS ? (char)tolower((int)c) : c;
			if(tmp == comp)
				return i;
		}

		return NET_STRING_NOT_FOUND;
	}

	size_t String::findLastOf(const char* pattern, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return NET_STRING_NOT_FOUND;

		const auto patternLen = strlen(pattern);

		size_t it = patternLen - 1;
		for (size_t i = size() - 1; i > 0; --i)
		{
			const auto tmp = (type & Net::String::type::NOT_CASE_SENS) ? (char)tolower((int)at(i)) : at(i);
			const auto comp = (type & Net::String::type::NOT_CASE_SENS) ? (char)tolower((int)pattern[it]) : pattern[it];
			if (tmp == comp)
			{
				if (it == 0)
					it = INVALID_SIZE;
				else
					it--;

				if (it == INVALID_SIZE)
				{
					return i;
				}
			}
			else
			{
				it = patternLen - 1;
			}
		}

		return NET_STRING_NOT_FOUND;
	}

	bool String::compare(char match, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		if (size() > 1)
			return false;

		if (type & NOT_CASE_SENS)
			return (this->operator[](0) == match);

		return ((char)tolower(this->operator[](0)) == (char)tolower(match));
	}

	bool String::compare(const char* match, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		auto len = strlen(match);

		if (this->size() != len)
			return false;

		for (size_t i = 0; i < len; ++i)
		{
			if (type & NOT_CASE_SENS)
			{
				if ((char)tolower(this->operator[](i)) != (char)tolower(match[i]))
					return false;
			}
			else
			{
				if (this->operator[](i) != match[i])
					return false;
			}
		}

		return true;
	}

	bool String::erase(const size_t len)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		if (len > size())
		{
			clear();
			return true;
		}

		/*
		* instead of allocating a new buffer
		* we use the method to move the characters up
		* and repos the null-terminator
		*/
		char prevChar = at(size() - 1);
		for (size_t i = 0; i < len; ++i)
		{
			for (size_t j = size() - 1; j > 0; --j)
			{
				char rem = at(j - 1);
				_string.set(j - 1, prevChar);
				prevChar = rem;
			}
		}
		_string.set(size() - len, '\0');
		_string.set_size(size() - len);

		/*
		* ok if we removed then we add the amount to _free_size
		* later we will use it for the append function
		*/
		if (_free_size == INVALID_FILE_SIZE)
		{
			_free_size = len;
		}
		else
		{
			_free_size += len;
		}

		return true;
	}

	bool String::erase(const size_t start, size_t len)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		if (start + len > size())
		{
			clear();
			return true;
		}

		/*
		* instead of allocating a new buffer
		* we use the method to move the characters up
		* and repos the null-terminator
		*/
		char prevChar = at(size() - 1);
		for (size_t i = 0; i < len; ++i)
		{
			for (size_t j = size() - 1; j > start; --j)
			{
				char rem = at(j - 1);
				_string.set(j - 1, prevChar);
				prevChar = rem;
			}
		}
		_string.set(size() - len, '\0');
		_string.set_size(size() - len);

		/*
		* ok if we removed then we add the amount to _free_size
		* later we will use it for the append function
		*/
		if (_free_size == INVALID_FILE_SIZE)
		{
			_free_size = len;
		}
		else
		{
			_free_size += len;
		}

		return true;
	}

	bool String::erase(const char c, const size_t start)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(start, c);
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, 0);
	}

	bool String::erase(const char c, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(c, type);
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, 0);
	}

	bool String::erase(const char c, const size_t start, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(start, c, type);
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, 0);
	}

	bool String::erase(const char* pattern, const size_t start)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(start, pattern);
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, strlen(pattern));
	}

	bool String::erase(const char* pattern, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(pattern, type);
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, strlen(pattern));
	}

	bool String::erase(const char* pattern, const size_t start, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(start, pattern, type);
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, strlen(pattern));
	}


	bool String::erase(String& pattern, const size_t start)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(start, pattern.get().data());
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, pattern.length());
	}

	bool String::erase(String& pattern, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(pattern.get().data(), type);
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, pattern.length());
	}

	bool String::erase(String& pattern, const size_t start, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		const auto it = find(start, pattern.get().data(), type);
		if (it == NET_STRING_NOT_FOUND)
			return false;

		return erase(it, pattern.length());
	}

	bool String::eraseAll(const char c, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		auto match = findAll(c, type);
		return eraseAll(match);
	}

	bool String::eraseAll(const char* pattern, const char type)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		auto match = findAll(pattern, type);
		return eraseAll(match);
	}

	bool Net::String::eraseAll(std::vector<size_t> m_IndexCharacterToSkip)
	{
		if (size() == INVALID_SIZE || size() == 0)
			return false;

		auto replaceSize = size() - m_IndexCharacterToSkip.size();
		auto replace = ALLOC<char>(replaceSize + 1);
		memset(replace, 0, replaceSize);
		replace[replaceSize] = 0;

		auto ref = get();
		auto dec = ref.get();
		for (size_t i = 0, j = 0; i < size(); ++i)
		{
			bool m_skip = false;
			for (auto& index : m_IndexCharacterToSkip)
			{
				if (i == index)
				{
					m_skip = true;
					break;
				}
			}
			if (m_skip) continue;

			replace[j] = dec[i];
			++j;
		}

		_string.free();
		_string = RUNTIMEXOR(replace);
		return true;
	}

	bool String::replace(const char c, const char r, const size_t start)
	{
		const auto i = find(start, c);
		if (i == NET_STRING_NOT_FOUND)
			return false;

		_string.set(i, r);
		return true;
	}

	bool String::replace(const char c, const char* r, const size_t start)
	{
		const auto i = find(start, c);
		if (i == NET_STRING_NOT_FOUND)
			return false;

		const auto str = revert();
		const auto rSize = strlen(r);
		const auto replaceSize = size() + rSize - 1;
		const auto replace = ALLOC<char>(replaceSize + 1);
		memcpy(replace, str.get(), size());
		memcpy(&replace[i], r, rSize);
		memcpy(&replace[i + rSize], &str.get()[i + 1], size() - i - 1);

		_string.free();
		_string = RUNTIMEXOR(replace);
		return true;
	}

	bool String::replace(const char* pattern, const char r, const size_t start)
	{
		const auto i = find(start, pattern);
		if (i == NET_STRING_NOT_FOUND)
			return false;

		const auto patternLen = strlen(pattern);
		const auto replaceSize = size() + 1;
		const auto replace = ALLOC<char>(replaceSize + 1);

		auto cinserted = false;
		size_t j = 0;
		for (size_t it = 0; it < size(); ++it)
		{
			if (it >= i && it < i + patternLen)
			{
				if (!cinserted)
				{
					memcpy(&replace[j], &r, 1);
					++j;
					cinserted = true;
				}
				continue;
			}

			auto tmp = at(it);
			memcpy(&replace[j], &tmp, 1);
			++j;
		}
		replace[replaceSize] = '\0';

		_string.free();
		_string = RUNTIMEXOR(replace);
		return true;
	}

	bool String::replace(const char* pattern, const char* r, const size_t start)
	{
		const auto i = find(start, pattern);
		if (i == NET_STRING_NOT_FOUND)
			return false;

		const auto patternLen = strlen(pattern);
		const auto rLen = strlen(r);
		const auto replaceSize = size() + rLen;
		const auto replace = ALLOC<char>(replaceSize + 1);

		auto cinserted = false;
		size_t j = 0;
		for (size_t it = 0; it < size(); ++it)
		{
			if (it >= i && it < i + patternLen)
			{
				if (!cinserted)
				{
					for (size_t x = 0; x < rLen; ++x)
					{
						memcpy(&replace[j], &r[x], 1);
						++j;
					}

					cinserted = true;
				}
				continue;
			}

			auto tmp = at(it);
			memcpy(&replace[j], &tmp, 1);
			++j;
		}
		replace[replaceSize] = '\0';

		_string.free();
		_string = RUNTIMEXOR(replace);
		return true;
	}

	bool String::replaceAll(const char c, const char r)
	{
		const auto found = findAll(c);
		if (found.empty())
			return false;

		for (auto& index : found)
			_string.set(index, r);

		return true;
	}

	bool String::replaceAll(const char c, const char* r)
	{
		const auto found = findAll(c);
		if (found.empty())
			return false;

		auto affectedOnce = false;
		for (size_t i = 0; i < size(); ++i)
		{
			const auto res = replace(c, r, i);
			if (!affectedOnce && res)
				affectedOnce = true;
		}

		return affectedOnce;
	}

	bool String::replaceAll(const char* pattern, const char r)
	{
		const auto found = findAll(pattern);
		if (found.empty())
			return false;

		auto affectedOnce = false;
		for (size_t i = 0; i < size(); ++i)
		{
			const auto res = replace(pattern, r, i);
			if (!affectedOnce && res)
				affectedOnce = true;
		}

		return affectedOnce;
	}

	bool String::replaceAll(const char* pattern, const char* r)
	{
		const auto found = findAll(pattern);
		if (found.empty())
			return false;

		auto affectedOnce = false;
		for (size_t i = 0; i < size(); ++i)
		{
			const auto res = replace(pattern, r, i);
			if (!affectedOnce && res)
				affectedOnce = true;
		}

		return affectedOnce;
	}
}
NET_POP