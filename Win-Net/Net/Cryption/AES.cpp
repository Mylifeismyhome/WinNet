#include "AES.h"

NET_NAMESPACE_BEGIN(Net)
NET_NAMESPACE_BEGIN(Cryption)
AES::AES()
{
	IV = nullptr;
	Key = nullptr;
	KeyLength = NULL;
}

AES::~AES()
{
	IV.free();
	Key.free();
}

bool AES::Init(char* in_Key, char* in_IV, const size_t in_KeyLength)
{
	if (!in_Key || !in_IV)
		return false;
	
	Key = RUNTIMEXOR(in_Key);
	IV = RUNTIMEXOR(in_IV);
	KeyLength = in_KeyLength;
	return true;
}

bool AES::encrypt(CryptoPP::byte* data, const size_t size, const char* key, const char* iv) const
{
	try
	{
		CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption encryptor(reinterpret_cast<const CryptoPP::byte*>(key), KeyLength, reinterpret_cast<const CryptoPP::byte*>(iv));
		encryptor.ProcessData(data, data, size);
		return true;
	}
	catch (const CryptoPP::Exception& ex)
	{
		LOG_ERROR(CSTRING("[NET_AES][ENCRPYT] - %s"), ex.what());
		return false;
	}
}

bool AES::encryptString(CryptoPP::byte* data, const size_t size)
{
	return encrypt(data, size, Key.Revert().get(), IV.Revert().get());
}

bool AES::encryptStringHex(CryptoPP::byte* data, CryptoPP::byte** out, size_t& size)
{
	if (encryptString(data, size))
	{
		const NET_HEX hex;
		return hex.encodeString(data, out, size);
	}

	return true;
}

bool AES::encryptStringHex(CryptoPP::byte** data, size_t& size)
{
	if (encryptString(*data, size))
	{
		const NET_HEX hex;
		return hex.encodeString(data, size);
	}

	return true;
}

bool AES::encryptStringBase64(CryptoPP::byte** data, size_t& size)
{
	if (encryptString(*data, size))
	{
		NET_BASE64 base64;
		base64.encodeString(data, size);
		return true;
	}

	return false;
}

bool AES::encryptStringBase64(CryptoPP::byte* data, CryptoPP::byte** out, size_t& size)
{
	if (encryptString(data, size))
	{
		NET_BASE64 base64;
		base64.encodeString(data, out, size);
		FREE(data);
		return true;
	}

	return false;
}

bool AES::decrypt(CryptoPP::byte* data, const size_t size, const char* key, const char* iv) const
{
	try
	{
		CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption decryptor(reinterpret_cast<const CryptoPP::byte*>(key), KeyLength, reinterpret_cast<const CryptoPP::byte*>(iv));
		decryptor.ProcessData(data, data, size);
		return true;
	}
	catch (const CryptoPP::Exception& ex)
	{
		LOG_ERROR(CSTRING("[NET_AES][DECRPYT] - %s"), ex.what());
		return false;
	}
}

bool AES::decryptString(CryptoPP::byte* data, const size_t size)
{
	return decrypt(data, size, Key.Revert().get(), IV.Revert().get());
}

bool AES::decryptStringHex(CryptoPP::byte* data, CryptoPP::byte** out, size_t& size)
{
	const NET_HEX hex;
	if (hex.decodeString(data, out, size))
	{
		if (decryptString(*out, size))
			return true;
	}

	return false;
}

bool AES::decryptStringHex(CryptoPP::byte** data, size_t& size)
{
	const NET_HEX hex;
	if (hex.decodeString(data, size))
	{
		if (decryptString(*data, size))
			return true;
	}

	return false;
}

bool AES::decryptStringBase64(CryptoPP::byte** data, size_t& size)
{
	NET_BASE64 base64;
	base64.decodeString(data, size);
	if (decryptString(*data, size))
		return true;

	return false;
}

bool AES::decryptStringBase64(CryptoPP::byte* data, CryptoPP::byte** out, size_t& size)
{
	NET_BASE64 base64;
	base64.decodeString(data, out, size);
	if (decryptString(*out, size))
	{
		FREE(data);
		return true;
	}

	return false;
}
NET_NAMESPACE_END
NET_NAMESPACE_END
