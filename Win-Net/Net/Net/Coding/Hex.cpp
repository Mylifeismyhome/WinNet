#include <Net/Coding/Hex.h>

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Coding)
static bool ProcessEncode(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
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

		FREE(out);
		out = encoded; // pointer swap
		out[hexSize] = '\0';

		size = hexSize;
		return true;
	}
	catch (const CryptoPP::Exception& ex)
	{
		NET_LOG_ERROR(CSTRING("[NET_HEX][ENCODE] - %s"), ex.what());
		return false;
	}
}

static bool ProcessEncode(CryptoPP::byte*& data, size_t& size)
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

		FREE(data);
		data = encoded; // pointer swap
		data[hexSize] = '\0';

		size = hexSize;
		return true;
	}
	catch (const CryptoPP::Exception& ex)
	{
		NET_LOG_ERROR(CSTRING("[NET_HEX][ENCODE] - %s"), ex.what());
		return false;
	}
}

static bool ProcessDecode(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
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
	catch (const CryptoPP::Exception& ex)
	{
		NET_LOG_ERROR(CSTRING("[NET_HEX][DECODE] - %s"), ex.what());
		return false;
	}
}

static bool ProcessDecode(CryptoPP::byte*& data, size_t& size)
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
	catch (const CryptoPP::Exception& ex)
	{
		NET_LOG_ERROR(CSTRING("[NET_HEX][DECODE] - %s"), ex.what());
		return false;
	}
}

bool Hex::encode(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
{
	return ProcessEncode(data, out, size);
}

bool Hex::encode(CryptoPP::byte*& data, size_t& size)
{
	return ProcessEncode(data, size);
}

bool Hex::decode(CryptoPP::byte* data, CryptoPP::byte*& out, size_t& size)
{
	return ProcessDecode(data, out, size);
}

bool Hex::decode(CryptoPP::byte*& data, size_t& size)
{
	return ProcessDecode(data, size);
}
NET_NAMESPACE_END
NET_NAMESPACE_END
