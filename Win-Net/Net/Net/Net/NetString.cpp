#include <Net/Net/NetString.h>

NET_IGNORE_CONVERSION_NULL
NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Pointer)
UniquePointer::UniquePointer(void* in)
{
	_pointer = in;
}

UniquePointer::~UniquePointer()
{
	FREE(_pointer);
}
NET_NAMESPACE_END

String::String()
{
	_string = nullptr;
	_size = 0;
}

String::String(const char in)
{
	Construct(in);
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

	Construct(str.data());
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

	_string = RUNTIMEXOR(str.data());
	_size = 1;
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

	_string = RUNTIMEXOR(str.data());
	_size = str.size();
}

void String::copy(String& in)
{
	_string.free();
	_string = RUNTIMEXOR(in.data().get());
	_size = in.size();
}

void String::move(String&& in)
{
	_string = RUNTIMEXOR(in.data().get());
	_size = in.size();
}

String::~String()
{
	_string.free();
}

size_t String::size() const
{
	return _size;
}

size_t String::length() const
{
	return _size - 1;
}

void String::set(const char in, ...)
{
	std::vector<char> str(2);
	str[0] = in;
	str[1] = '\0';

	_string.free();
	_string = RUNTIMEXOR(str.data());
	_size = 1;
}

void String::append(const char in)
{
	if (size() <= 0)
	{
		Construct(in);
		return;
	}

	CPOINTER<byte> data(ALLOC<byte>(_size + 1));
	memcpy(&data.get()[0], _string.revert().get(), _size - 1);
	memcpy(&data.get()[_size - 1], &in, 1);
	data.get()[_size] = '\0';

	_string.free();
	_string = RUNTIMEXOR(reinterpret_cast<char*>(data.get()));
	_size = _size + 1;

	data.free();
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

	_string.free();
	_string = RUNTIMEXOR(str.data());
	_size = str.size();
}

void String::append(const char* in, ...)
{
	if (size() <= 0)
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

	CPOINTER<byte> data(ALLOC<byte>(_size + str.size() + 1));
	memcpy(&data.get()[0], _string.revert().get(), _size - 1);
	memcpy(&data.get()[_size - 1], str.data(), str.size());
	data.get()[_size + str.size()] = '\0';

	_string.free();
	_string = RUNTIMEXOR(reinterpret_cast<char*>(data.get()));
	_size = _size + str.size() - 1;

	data.free();
}

void String::set(String& in, ...)
{
	char* ref = in.get().get();
	va_list vaArgs;

#ifdef BUILD_LINUX
	va_start(vaArgs, ref);
	const size_t size = std::vsnprintf(nullptr, 0, ref, vaArgs);
	va_end(vaArgs);

	va_start(vaArgs, ref);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), ref, vaArgs);
	va_end(vaArgs);
#else
	va_start(vaArgs, ref);
	const size_t size = std::vsnprintf(nullptr, 0, ref, vaArgs);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), ref, vaArgs);
	va_end(vaArgs);
#endif

	_string.free();
	_string = RUNTIMEXOR(str.data());
	_size = str.size();
}

void String::append(String& in, ...)
{
	if (size() <= 0)
	{
		copy(in);
		return;
	}

	char* ref = in.get().get();
	va_list vaArgs;

#ifdef BUILD_LINUX
	va_start(vaArgs, ref);
	const size_t size = std::vsnprintf(nullptr, 0, ref, vaArgs);
	va_end(vaArgs);

	va_start(vaArgs, ref);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), ref, vaArgs);
	va_end(vaArgs);
#else
	va_start(vaArgs, ref);
	const size_t size = std::vsnprintf(nullptr, 0, ref, vaArgs);
	std::vector<char> str(size + 1);
	std::vsnprintf(str.data(), str.size(), ref, vaArgs);
	va_end(vaArgs);
#endif

	CPOINTER<byte> data(ALLOC<byte>(_size + str.size() + 1));
	memcpy(&data.get()[0], _string.revert().get(), _size - 1);
	memcpy(&data.get()[_size - 1], str.data(), str.size());
	data.get()[_size + str.size()] = '\0';

	_string.free();
	_string = RUNTIMEXOR(reinterpret_cast<char*>(data.get()));
	_size = _size + str.size() - 1;

	data.free();
}

Net::Cryption::XOR_UNIQUEPOINTER String::str()
{
	if (size() <= 0)
		return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

	return _string.revert();
}

Net::Cryption::XOR_UNIQUEPOINTER String::cstr()
{
	if (size() <= 0)
		return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

	return _string.revert();
}

Net::Cryption::XOR_UNIQUEPOINTER String::get()
{
	if (size() <= 0)
		return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

	return _string.revert();
}

Net::Cryption::XOR_UNIQUEPOINTER String::revert()
{
	if (size() <= 0)
		return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

	return _string.revert();
}

Net::Cryption::XOR_UNIQUEPOINTER String::data()
{
	if (size() <= 0)
		return Net::Cryption::XOR_UNIQUEPOINTER(nullptr, NULL, false);

	return _string.revert();
}

void String::clear()
{
	_string.free();
	_size = 0;
}


bool String::empty()
{
	if (size() <= 0)
		return true;

	const auto buffer = revert();
	const auto e = !memcmp(buffer.get(), "", size());
	return e;
}

char String::at(const size_t i)
{
	if (size() <= 0)
		return '\0';

	if (i > length())
		return '\0';

	const auto buffer = revert();
	const auto ch = buffer.get()[i];
	return ch;
}

char* String::substr(size_t length)
{
	if (size() <= 0)
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
	if (size() <= 0)
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

size_t String::find(char c)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	for (size_t i = 0; i < size(); ++i)
	{
		const auto tmp = at(i);
		if (!memcmp(&tmp, &c, 1))
			return i;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::find(const char c, const char type)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	for (size_t i = 0; i < size(); ++i)
	{
		const auto tmp = type & NOT_CASE_SENS ? (char)tolower((int)at(i)) : at(i);
		const auto comp = type & NOT_CASE_SENS ? (char)tolower((int)c) : c;
		if (!memcmp(&tmp, &comp, 1))
			return i;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::find(const char* pattern)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	const auto patternLen = strlen(pattern);

	size_t it = 0;
	for (size_t i = 0; i < size(); ++i)
	{
		const auto tmp = at(i);
		if (!memcmp(&tmp, &pattern[it], 1))
		{
			it++;

			if (it == patternLen)
				return i - it + 1;
		}
		else
			it = 0;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::find(const char* pattern, const char type)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	const auto patternLen = strlen(pattern);

	auto tmpMatch = ALLOC<char>(patternLen + 1);
	memset(tmpMatch, NULL, patternLen);
	if (type & NOT_CASE_SENS)
	{
		for (size_t i = 0; i < patternLen; ++i)
			tmpMatch[i] = (char)tolower((int)pattern[i]);
	}
	tmpMatch[patternLen] = '\0';

	size_t it = 0;
	for (size_t i = 0; i < size(); ++i)
	{
		const auto tmp = (char)tolower((int)at(i));
		if (!memcmp(&tmp, type & NOT_CASE_SENS ? &tmpMatch[it] : &pattern[it], 1))
		{
			it++;

			if (it == patternLen)
			{
				FREE(tmpMatch);
				return i - it + 1;
			}
		}
		else
			it = 0;
	}

	FREE(tmpMatch);
	return NET_STRING_NOT_FOUND;
}

size_t String::find(const size_t start, const char pattern)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	if (start > length())
		return NET_STRING_NOT_FOUND;

	const auto patternLen = 1;

	size_t it = 0;
	for (auto i = start; i < size(); ++i)
	{
		const auto tmp = at(i);
		if (!memcmp(&tmp, &pattern, 1))
		{
			it++;

			if (it == patternLen)
				return i - it + 1;
		}
		else
			it = 0;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::find(const size_t start, char pattern, const char type)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	if (start > length())
		return NET_STRING_NOT_FOUND;

	const auto patternLen = 1;

	if (type & NOT_CASE_SENS)
		pattern = (char)tolower((int)pattern);

	size_t it = 0;
	for (auto i = start; i < size(); ++i)
	{
		const auto tmp = (char)tolower((int)at(i));
		if (!memcmp(&tmp, &pattern, 1))
		{
			it++;

			if (it == patternLen)
				return i - it + 1;
		}
		else
			it = 0;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::find(const size_t start, char* pattern)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	if (start > length())
		return NET_STRING_NOT_FOUND;

	const auto patternLen = strlen(pattern);

	size_t it = 0;
	for (auto i = start; i < size(); ++i)
	{
		const auto tmp = at(i);
		if (!memcmp(&tmp, &pattern[it], 1))
		{
			it++;

			if (it == patternLen)
				return i - it + 1;
		}
		else
			it = 0;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::find(const size_t start, char* pattern, const char type)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	if (start > length())
		return NET_STRING_NOT_FOUND;

	const auto patternLen = strlen(pattern);

	auto tmpMatch = ALLOC<char>(patternLen + 1);
	memset(tmpMatch, NULL, patternLen);
	if (type & NOT_CASE_SENS)
	{
		for (size_t i = 0; i < patternLen; ++i)
			tmpMatch[i] = (char)tolower((int)pattern[i]);
	}
	tmpMatch[patternLen] = '\0';

	size_t it = 0;
	for (auto i = start; i < size(); ++i)
	{
		const auto tmp = (char)tolower((int)at(i));
		if (!memcmp(&tmp, type & NOT_CASE_SENS ? &tmpMatch[it] : &pattern[it], 1))
		{
			it++;

			if (it == patternLen)
			{
				FREE(tmpMatch);
				return i - it + 1;
			}
		}
		else
			it = 0;
	}

	FREE(tmpMatch);
	return NET_STRING_NOT_FOUND;
}

size_t String::find(const size_t start, const char* pattern)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	if (start > length())
		return NET_STRING_NOT_FOUND;

	const auto patternLen = strlen(pattern);

	size_t it = 0;
	for (auto i = start; i < size(); ++i)
	{
		const auto tmp = at(i);
		if (!memcmp(&tmp, &pattern[it], 1))
		{
			it++;

			if (it == patternLen)
				return i - it + 1;
		}
		else
			it = 0;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::find(const size_t start, const char* pattern, const char type)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	if (start > length())
		return NET_STRING_NOT_FOUND;

	const auto patternLen = strlen(pattern);

	auto tmpMatch = ALLOC<char>(patternLen + 1);
	memset(tmpMatch, NULL, patternLen);
	if (type & NOT_CASE_SENS)
	{
		for (size_t i = 0; i < patternLen; ++i)
			tmpMatch[i] = (char)tolower((int)pattern[i]);
	}
	tmpMatch[patternLen] = '\0';

	size_t it = 0;
	for (auto i = start; i < size(); ++i)
	{
		const auto tmp = (char)tolower((int)at(i));
		if (!memcmp(&tmp, type & NOT_CASE_SENS ? &tmpMatch[it] : &pattern[it], 1))
		{
			it++;

			if (it == patternLen)
			{
				FREE(tmpMatch);
				return i - it + 1;
			}
		}
		else
			it = 0;
	}

	FREE(tmpMatch);
	return NET_STRING_NOT_FOUND;
}

std::vector<size_t> String::findAll(const char c)
{
	std::vector<size_t> tmp;

	if (size() <= 0)
		return tmp;

	for (size_t i = 0; i < size(); ++i)
	{
		const auto res = find(i, c);
		if (res != NET_STRING_NOT_FOUND)
			tmp.emplace_back(res);
	}

	return tmp;
}

std::vector<size_t> String::findAll(const char c, const char type)
{
	std::vector<size_t> tmp;

	if (size() <= 0)
		return tmp;

	for (size_t i = 0; i < size(); ++i)
	{
		const auto res = find(i, c, type);
		if (res != NET_STRING_NOT_FOUND)
			tmp.emplace_back(res);
	}

	return tmp;
}

std::vector<size_t> String::findAll(const char* pattern)
{
	std::vector<size_t> tmp;

	if (size() <= 0)
		return tmp;

	for (size_t i = 0; i < size(); ++i)
	{
		const auto res = find(i, pattern);
		if (res != NET_STRING_NOT_FOUND)
			tmp.emplace_back(res);
	}

	return tmp;
}

std::vector<size_t> String::findAll(const char* pattern, const char type)
{
	std::vector<size_t> tmp;

	if (size() <= 0)
		return tmp;

	for (size_t i = 0; i < size(); ++i)
	{
		const auto res = find(i, pattern, type);
		if (res != NET_STRING_NOT_FOUND)
			tmp.emplace_back(res);
	}

	return tmp;
}

size_t String::findLastOf(char c)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	for (size_t i = size() - 1; i > 0; --i)
	{
		const auto tmp = at(i);
		if (!memcmp(&tmp, &c, 1))
			return i;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::findLastOf(const char c, const char type)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	for (size_t i = size() - 1; i > 0; --i)
	{
		const auto tmp = type & NOT_CASE_SENS ? (char)tolower((int)at(i)) : at(i);
		const auto comp = type & NOT_CASE_SENS ? (char)tolower((int)c) : c;
		if (!memcmp(&tmp, &comp, 1))
			return i;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::findLastOf(const char* pattern)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	const auto patternLen = strlen(pattern);

	size_t it = patternLen - 1;
	for (size_t i = size() - 1; i > 0; --i)
	{
		const auto tmp = at(i);
		if (!memcmp(&tmp, &pattern[it], 1))
		{
			if (it == 0)
				it = INVALID_SIZE;
			else
				it--;

			if (it == INVALID_SIZE)
				return i;
		}
		else
			it = patternLen - 1;
	}

	return NET_STRING_NOT_FOUND;
}

size_t String::findLastOf(const char* pattern, const char type)
{
	if (size() <= 0)
		return NET_STRING_NOT_FOUND;

	const auto patternLen = strlen(pattern);

	auto tmpMatch = ALLOC<char>(patternLen + 1);
	memset(tmpMatch, NULL, patternLen);
	if (type & NOT_CASE_SENS)
	{
		for (size_t i = 0; i < patternLen; ++i)
			tmpMatch[i] = (char)tolower((int)pattern[i]);
	}
	tmpMatch[patternLen] = '\0';

	size_t it = patternLen - 1;
	for (size_t i = size() - 1; i > 0; --i)
	{
		const auto tmp = (char)tolower((int)at(i));
		if (!memcmp(&tmp, type & NOT_CASE_SENS ? &tmpMatch[it] : &pattern[it], 1))
		{
			if (it == 0)
				it = INVALID_SIZE;
			else
				it--;

			if (it == INVALID_SIZE)
			{
				FREE(tmpMatch);
				return i;
			}
		}
		else
			it = patternLen - 1;
	}

	FREE(tmpMatch);
	return NET_STRING_NOT_FOUND;
}

bool String::compare(const char match)
{
	if (size() <= 0)
		return false;

	const auto buffer = revert();
	const auto cmp = !memcmp(buffer.get(), &match, size());
	return cmp;
}

bool String::compare(char match, const char type)
{
	if (size() <= 0)
		return false;

	const auto tmp = revert();
	size_t len = 1;
	if (type & NOT_CASE_SENS)
	{
		match = (char)tolower((int)match);

		for (size_t i = 0; i < size(); ++i)
			tmp.get()[i] = (char)tolower((int)tmp.get()[i]);
	}

	if (type & IN_LEN)
	{
		if (len > size())
			len = size();

		const auto cmp = !memcmp(tmp.get(), &match, len);
		return cmp;
	}

	const auto cmp = !memcmp(tmp.get(), &match, size());
	return cmp;
}

bool String::compare(const char* match)
{
	if (size() <= 0)
		return false;

	const auto buffer = revert();
	const auto cmp = !memcmp(buffer.get(), match, size());
	return cmp;
}

bool String::compare(const char* match, const char type)
{
	if (size() <= 0)
		return false;

	const auto tmp = revert();
	auto len = strlen(match);
	auto tmpMatch = ALLOC<char>(len + 1);
	memset(tmpMatch, NULL, len);
	if (type & NOT_CASE_SENS)
	{
		for (size_t i = 0; i < len; ++i)
			tmpMatch[i] = (char)tolower((int)match[i]);

		for (size_t i = 0; i < size(); ++i)
			tmp.get()[i] = (char)tolower((int)tmp.get()[i]);
	}
	tmpMatch[len] = '\0';

	if (type & IN_LEN)
	{
		if (len > size())
			len = size();

		const auto res = !memcmp(tmp.get(), type & NOT_CASE_SENS ? tmpMatch : match, len);
		FREE(tmpMatch);
		return res;
	}

	const auto res = !memcmp(tmp.get(), type & NOT_CASE_SENS ? tmpMatch : match, size());
	FREE(tmpMatch);
	return res;
}

bool String::compare(char* match)
{
	if (size() <= 0)
		return false;

	const auto buffer = revert();
	const auto cmp = !memcmp(buffer.get(), match, size());
	return cmp;
}

bool String::compare(char* match, const char type)
{
	if (size() <= 0)
		return false;

	const auto tmp = revert();
	auto len = strlen(match);
	if (type & NOT_CASE_SENS)
	{
		for (size_t i = 0; i < len; ++i)
			match[i] = (char)tolower((int)match[i]);

		for (size_t i = 0; i < size(); ++i)
			tmp.get()[i] = (char)tolower((int)tmp.get()[i]);
	}

	if (type & IN_LEN)
	{
		if (len > size())
			len = size();

		const auto cmp = !memcmp(tmp.get(), match, len);
		return cmp;
	}

	const auto cmp = !memcmp(tmp.get(), match, size());
	return cmp;
}

bool String::erase(const size_t len)
{
	if (size() <= 0)
		return false;

	if (len > size() - len)
	{
		clear();
		return true;
	}

	const auto replace = ALLOC<char>(len + 1);
	for (size_t i = len, it = 0; i < size(); ++i, ++it)
	{
		const auto tmp = at(i);
		memcpy(&replace[it], &tmp, 1);
	}
	replace[size() - len] = '\0';

	_string.free();
	_string = RUNTIMEXOR(replace);
	_size = size() - len;
	return true;
}

bool String::erase(const size_t start, size_t len)
{
	if (size() <= 0)
		return false;

	if (start + len >= length())
		len = length() - start;

	const auto replaceSize = size() - len;
	const auto replace = ALLOC<char>(replaceSize + 1);
	for (size_t i = 0, j = 0; i < size(); ++i)
	{
		if (i >= start && i <= start + len - 1)
			continue;

		const auto tmp = at(i);
		memcpy(&replace[j], &tmp, 1);
		++j;
	}
	replace[replaceSize] = '\0';

	_string.free();
	_string = RUNTIMEXOR(replace);
	_size = replaceSize;
	return true;
}

bool String::erase(const char c, const size_t start)
{
	if (size() <= 0)
		return false;

	const auto it = find(start, c);
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, 0);
}

bool String::erase(const char c, const char type)
{
	if (size() <= 0)
		return false;

	const auto it = find(c, type);
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, 0);
}

bool String::erase(const char c, const size_t start, const char type)
{
	if (size() <= 0)
		return false;

	const auto it = find(start, c, type);
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, 0);
}

bool String::erase(const char* pattern, const size_t start)
{
	if (size() <= 0)
		return false;

	const auto it = find(start, pattern);
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, strlen(pattern));
}

bool String::erase(const char* pattern, const char type)
{
	if (size() <= 0)
		return false;

	const auto it = find(pattern, type);
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, strlen(pattern));
}

bool String::erase(const char* pattern, const size_t start, const char type)
{
	if (size() <= 0)
		return false;

	const auto it = find(start, pattern, type);
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, strlen(pattern));
}


bool String::erase(String& pattern, const size_t start)
{
	if (size() <= 0)
		return false;

	const auto it = find(start, pattern.get().data());
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, pattern.length());
}

bool String::erase(String& pattern, const char type)
{
	if (size() <= 0)
		return false;

	const auto it = find(pattern.get().data(), type);
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, pattern.length());
}

bool String::erase(String& pattern, const size_t start, const char type)
{
	if (size() <= 0)
		return false;

	const auto it = find(start, pattern.get().data(), type);
	if (it == NET_STRING_NOT_FOUND)
		return false;

	return erase(it, pattern.length());
}

bool String::eraseAll(const char c)
{
	if (size() <= 0)
		return false;

	std::vector<size_t> tmp;
	size_t it = 0;
	auto res = NET_STRING_NOT_FOUND;
	do
	{
		res = find(it, c);
		if (res != NET_STRING_NOT_FOUND)
		{
			tmp.emplace_back(res);
			it = res + 1;
		}
	} while (res != NET_STRING_NOT_FOUND);

	if (tmp.empty())
		return false;

	const auto replaceSize = size() - tmp.size();
	const auto replace = ALLOC<char>(replaceSize + 1);

	for (size_t i = 0, j = 0; i < size(); ++i)
	{
		auto skip = false;
		for (auto& value : tmp)
		{
			if (value == i)
			{
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		const auto ch = at(i);
		memcpy(&replace[j], &ch, 1);
		++j;
	}

	_string.free();
	_string = RUNTIMEXOR(replace);
	_size = replaceSize;
	return true;
}

bool String::eraseAll(const char c, const char type)
{
	if (size() <= 0)
		return false;

	std::vector<size_t> tmp;
	size_t it = 0;
	auto res = NET_STRING_NOT_FOUND;
	do
	{
		res = find(it, c, type);
		if (res != NET_STRING_NOT_FOUND)
		{
			tmp.emplace_back(res);
			it = res + 1;
		}
	} while (res != NET_STRING_NOT_FOUND);

	if (tmp.empty())
		return false;

	const auto replaceSize = size() - tmp.size();
	const auto replace = ALLOC<char>(replaceSize + 1);

	for (size_t i = 0, j = 0; i < size(); ++i)
	{
		auto skip = false;
		for (auto& value : tmp)
		{
			if (value == i)
			{
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		const auto ch = at(i);
		memcpy(&replace[j], &ch, 1);
		++j;
	}

	_string.free();
	_string = RUNTIMEXOR(replace);
	_size = replaceSize;
	return true;
}

bool String::eraseAll(const char* pattern)
{
	if (size() <= 0)
		return false;

	std::vector<size_t> tmp;
	size_t it = 0;
	auto res = NET_STRING_NOT_FOUND;
	do
	{
		res = find(it, pattern);
		if (res != NET_STRING_NOT_FOUND)
		{
			tmp.emplace_back(res);
			it = res + 1;
		}
	} while (res != NET_STRING_NOT_FOUND);

	if (tmp.empty())
		return false;

	const auto replaceSize = size() - tmp.size();
	const	 auto replace = ALLOC<char>(replaceSize + 1);
	const auto plen = strlen(pattern);

	for (size_t i = 0, j = 0; i < size(); ++i)
	{
		auto skip = false;
		for (auto& value : tmp)
		{
			if (i >= value && i < value + plen)
			{
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		const auto ch = at(i);
		memcpy(&replace[j], &ch, 1);
		++j;
	}

	_string.free();
	_string = RUNTIMEXOR(replace);
	_size = replaceSize;
	return true;
}

bool String::eraseAll(const char* pattern, const char type)
{
	if (size() <= 0)
		return false;

	std::vector<size_t> tmp;
	size_t it = 0;
	auto res = NET_STRING_NOT_FOUND;
	do
	{
		res = find(it, pattern, type);
		if (res != NET_STRING_NOT_FOUND)
		{
			tmp.emplace_back(res);
			it = res + 1;
		}
	} while (res != NET_STRING_NOT_FOUND);

	if (tmp.empty())
		return false;

	const auto replaceSize = size() - tmp.size();
	const auto replace = ALLOC<char>(replaceSize + 1);
	const auto plen = strlen(pattern);

	for (size_t i = 0, j = 0; i < size(); ++i)
	{
		auto skip = false;
		for (auto& value : tmp)
		{
			if (i >= value && i < value + plen)
			{
				skip = true;
				break;
			}
		}
		if (skip)
			continue;

		const auto ch = at(i);
		memcpy(&replace[j], &ch, 1);
		++j;
	}

	_string.free();
	_string = RUNTIMEXOR(replace);
	_size = replaceSize;
	return true;
}

bool String::replace(const char c, const char r, const size_t start)
{
	const auto i = find(start, c);
	if (i == NET_STRING_NOT_FOUND)
		return false;

	const auto str = revert();
	str.get()[i] = r;
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
	_size = replaceSize;
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
	_size = replaceSize;
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
	_size = replaceSize;
	return true;
}

bool String::replaceAll(const char c, const char r)
{
	const auto found = findAll(c);
	if (found.empty())
		return false;

	const auto str = revert();
	for (auto& val : found)
		str.get()[val] = r;

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
NET_NAMESPACE_END
NET_POP
