#include "BASE64.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
bool Base64::encode(BYTE* data, BYTE** out, size_t& size) const
{
	const auto encoded = Base64_Encode(data, size, &size);
	if (!encoded)
		return false;

	data = encoded; // pointer swap
	data[size] = '\0';

	return true;
}

bool Base64::encode(BYTE** data, size_t& size) const
{
	const auto encoded = Base64_Encode(*data, size, &size);
	if (!encoded)
		return false;

	*data = encoded; // pointer swap
	data[0][size] = '\0';

	return true;
}

bool Base64::encodeString(BYTE* data, BYTE** out, size_t& size) const
{
	return encode(data, out, size);
}

bool Base64::encodeString(BYTE** data, size_t& size) const
{
	return encode(data, size);
}

bool Base64::decode(BYTE* data, BYTE** out, size_t& size) const
{
	const auto decoded = Base64_Decode(data, size, &size);
	if (!decoded)
		return false;

	data = decoded; // pointer swap
	data[size] = '\0';

	return true;
}

bool Base64::decode(BYTE** data, size_t& size) const
{
	const auto decoded = Base64_Decode(*data, size, &size);
	if (!decoded)
		return false;

	*data = decoded; // pointer swap
	data[0][size] = '\0';

	return true;
}

bool Base64::decodeString(BYTE* data, BYTE** out, size_t& size) const
{
	return decode(data, out, size);
}

bool Base64::decodeString(BYTE** data, size_t& size) const
{
	return decode(data, size);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
