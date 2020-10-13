#include "BASE64.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
static bool ProcessEncode(BYTE* data, BYTE*& out, size_t& size)
{
	const auto encoded = Base64_Encode(data, size, &size);
	if (!encoded)
		return false;

	data = encoded; // pointer swap
	data[size] = '\0';

	return true;
}

static bool ProcessEncode(BYTE*& data, size_t& size)
{
	const auto encoded = Base64_Encode(data, size, &size);
	if (!encoded)
		return false;

	data = encoded; // pointer swap
	data[size] = '\0';

	return true;
}

static bool ProcessDecode(BYTE* data, BYTE*& out, size_t& size)
{
	const auto decoded = Base64_Decode(data, size, &size);
	if (!decoded)
		return false;

	data = decoded; // pointer swap
	data[size] = '\0';

	return true;
}

static bool ProcessDecode(BYTE*& data, size_t& size)
{
	const auto decoded = Base64_Decode(data, size, &size);
	if (!decoded)
		return false;

	data = decoded; // pointer swap
	data[size] = '\0';

	return true;
}

bool Base64::encode(BYTE* data, BYTE*& out, size_t& size) const
{
	return ProcessEncode(data, out, size);
}

bool Base64::encode(BYTE*& data, size_t& size) const
{
	return ProcessEncode(data, size);
}

bool Base64::decode(BYTE* data, BYTE*& out, size_t& size) const
{
	return ProcessDecode(data, out, size);
}

bool Base64::decode(BYTE*& data, size_t& size) const
{
	return ProcessDecode(data, size);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
