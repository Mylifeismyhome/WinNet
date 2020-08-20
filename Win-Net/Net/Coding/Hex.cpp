#include "HEX.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
bool HEX::encode(CryptoPP::byte* data, CryptoPP::byte** out, size_t& size) const
{
	try
	{
		const auto hexSize = (size * 2);
		CryptoPP::HexEncoder encoder;
		encoder.Put(data, size);
		encoder.MessageEnd();
		const auto encoded = ALLOC<byte>(hexSize + 1);
		encoder.Get(encoded, hexSize);
		encoded[hexSize] = '\0';

		FREE(*out);
		*out = encoded; // pointer swap
		out[0][hexSize] = '\0';

		size = hexSize;
		return true;
	}
	catch (const CryptoPP::Exception & ex)
	{
		LOG_ERROR(CSTRING("[NET_HEX][ENCODE] - %s"), ex.what());
		return false;
	}
}

bool HEX::encode(CryptoPP::byte** data, size_t& size) const
{
	try
	{
		const auto hexSize = (size * 2);
		CryptoPP::HexEncoder encoder;
		encoder.Put(*data, size);
		encoder.MessageEnd();
		const auto encoded = ALLOC<byte>(hexSize + 1);
		encoder.Get(encoded, hexSize);
		encoded[hexSize] = '\0';

		FREE(*data);
		*data = encoded; // pointer swap
		data[0][hexSize] = '\0';
		
		size = hexSize;
		return true;
	}
	catch (const CryptoPP::Exception & ex)
	{
		LOG_ERROR(CSTRING("[NET_HEX][ENCODE] - %s"), ex.what());
		return false;
	}
}

bool HEX::encodeString(CryptoPP::byte* data, CryptoPP::byte** out, size_t& size) const
{
	return encode(data, out, size);
}

bool HEX::encodeString(CryptoPP::byte** data, size_t& size) const
{
	return encode(data, size);
}

bool HEX::decode(CryptoPP::byte* data, CryptoPP::byte** out, size_t& size) const
{
	try
	{
		const auto originalSize = (size / 2);
		CryptoPP::HexDecoder decoder;
		decoder.Put(data, size);
		decoder.MessageEnd();
		const auto decoded = ALLOC<byte>(originalSize + 1);
		decoder.Get(decoded, originalSize);
		decoded[originalSize] = '\0';

		FREE(data);
		data = decoded; // pointer swap
		data[originalSize] = '\0';

		size = originalSize;
		return true;
	}
	catch (const CryptoPP::Exception & ex)
	{
		LOG_ERROR(CSTRING("[NET_HEX][DECODE] - %s"), ex.what());
		return false;
	}
}

bool HEX::decode(CryptoPP::byte** data, size_t& size) const
{
	try
	{
		const auto originalSize = (size / 2);
		CryptoPP::HexDecoder decoder;
		decoder.Put(*data, size);
		decoder.MessageEnd();
		const auto decoded = ALLOC<byte>(originalSize + 1);
		decoder.Get(decoded, originalSize);
		decoded[originalSize] = '\0';

		FREE(*data);
		*data = decoded; // pointer swap
		data[0][originalSize] = '\0';

		size = originalSize;
		return true;
	}
	catch (const CryptoPP::Exception & ex)
	{
		LOG_ERROR(CSTRING("[NET_HEX][DECODE] - %s"), ex.what());
		return false;
	}
}

bool HEX::decodeString(CryptoPP::byte* data, CryptoPP::byte** out, size_t& size) const
{
	return decode(data, out, size);
}

bool HEX::decodeString(CryptoPP::byte** data, size_t& size) const
{
	return decode(data, size);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
